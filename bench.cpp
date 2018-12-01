#include <xbyak.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <cpuid.h>
#include <string.h>

static bool output_csv = false;
static FILE *logs;

/* x64 regisuter usage
 *  http://msdn.microsoft.com/en-US/library/9z1stfyw(v=vs.80).aspx
 * RAX          Volatile Return       value register
 * RCX          Volatile              First integer argument
 * RDX          Volatile              Second integer argument
 * R8           Volatile              Third integer argument
 * R9           Volatile              Fourth integer argument
 * R10:R11      Volatile              Must be preserved as needed by caller; used in syscall/sysret instructions
 * R12:R15      Nonvolatile           Must be preserved by callee
 * RDI          Nonvolatile           Must be preserved by callee
 * RSI          Nonvolatile           Must be preserved by callee
 * RBX          Nonvolatile           Must be preserved by callee
 * RBP          Nonvolatile           May be used as a frame pointer; must be preserved by callee
 * RSP          Nonvolatile           Stack pointer
 * XMM0         Volatile              First FP argument
 * XMM1         Volatile              Second FP argument
 * XMM2         Volatile              Third FP argument
 * XMM3         Volatile              Fourth FP argument
 * XMM4:XMM5    Volatile              Must be preserved as needed by caller
 * XMM6:XMM15   Nonvolatile           Must be preserved as needed by callee.
 */

#ifdef __linux
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <sys/eventfd.h>

static int
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags )
{
    int ret;

    ret = syscall( __NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags );
    return ret;
}

int perf_fd;

static void
cycle_counter_init(void)
{
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));

    attr.type = PERF_TYPE_HARDWARE;
    attr.size = sizeof(attr);
    attr.config = PERF_COUNT_HW_CPU_CYCLES;
    attr.exclude_kernel = 1;

    perf_fd = perf_event_open(&attr, 0, -1, -1, 0);
    if (perf_fd == -1) {
        perror("perf_event_open");
        exit(1);
    }
}

static long long
read_cycle(void)
{
    long long val;
    ssize_t sz = read(perf_fd, &val, sizeof(val));
    if (sz != sizeof(val)) {
        perror("read");
        exit(1);
    }

    return val;
}

#else

#define cycle_counter_init() ((void)0)
#define read_cycle() __rdtsc()

#endif

char MIE_ALIGN(2048*1024) zero_mem[4096*1024];
char MIE_ALIGN(2048*1024) data_mem[4096*1024];

enum lt_op {
    LT_LATENCY,
    LT_THROUGHPUT,
    LT_THROUGHPUT_KILLDEP
};

enum operand_type {
    OT_INT,
    OT_FP32,
    OT_FP64
};

template <typename T> struct RegMap;

template <>
struct RegMap<Xbyak::Xmm>
{
    const char *name;
    Xbyak::Xmm v4, v5, v6, v7;
    Xbyak::Xmm v8, v9, v10, v11, v12, v13, v14, v15;

    RegMap()
        :name("m128"),
         v4(4), v5(5), v6(6), v7(7),
         v8(8), v9(9), v10(10), v11(11), v12(12), v13(13), v14(14), v15(15)
        {}

    bool vec_reg() {
        return true;
    }

    void save(Xbyak::CodeGenerator *g, Xbyak::Xmm r, int off, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->movdqa(g->ptr [g->rsp + off], r);
            break;
        case OT_FP32:
            g->movaps(g->ptr [g->rsp + off], r);
            break;
        case OT_FP64:
            g->movapd(g->ptr [g->rsp + off], r);
            break;
        }
    }

    void restore(Xbyak::CodeGenerator *g, Xbyak::Xmm r, int off, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->movdqa(r, g->ptr [g->rsp + off]);
            break;
        case OT_FP32:
            g->movaps(r, g->ptr [g->rsp + off]);
            break;
        case OT_FP64:
            g->movapd(r, g->ptr [g->rsp + off]);
            break;
        }
    }

    void killdep(Xbyak::CodeGenerator *g, Xbyak::Xmm r, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->pxor(r, r);
            break;
        case OT_FP32:
            g->xorps(r, r);
            break;
        case OT_FP64:
            g->xorpd(r, r);
            break;
        }
    }
};

template <>
struct RegMap<Xbyak::Ymm>
{
    const char *name;

    Xbyak::Ymm v4, v5, v6, v7;
    Xbyak::Ymm v8, v9, v10, v11, v12, v13, v14, v15;

    RegMap()
        :name("m256"),
         v4(4), v5(5), v6(6), v7(7),
         v8(8), v9(9), v10(10), v11(11), v12(12), v13(13), v14(14), v15(15)
        {}

    bool vec_reg() {
        return true;
    }

    void save(Xbyak::CodeGenerator *g, Xbyak::Ymm r, int off, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->vmovdqa(g->ptr [g->rsp + off], r);
            break;
        case OT_FP32:
            g->vmovaps(g->ptr [g->rsp + off], r);
            break;
        case OT_FP64:
            g->vmovapd(g->ptr [g->rsp + off], r);
            break;
        }
    }

    void restore(Xbyak::CodeGenerator *g, Xbyak::Ymm r, int off, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->vmovdqa(r, g->ptr [g->rsp + off]);
            break;
        case OT_FP32:
            g->vmovaps(r, g->ptr [g->rsp + off]);
            break;
        case OT_FP64:
            g->vmovapd(r, g->ptr [g->rsp + off]);
            break;
        }
    }

    void killdep(Xbyak::CodeGenerator *g, Xbyak::Ymm r, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->vpxor(r, r, r);
            break;
        case OT_FP32:
            g->vxorps(r, r, r);
            break;
        case OT_FP64:
            g->vxorpd(r, r, r);
            break;
        }
    }
};

template <>
struct RegMap<Xbyak::Zmm>
{
    const char *name;
    Xbyak::Zmm v4, v5, v6, v7;
    Xbyak::Zmm v8, v9, v10, v11, v12, v13, v14, v15;
    Xbyak::Zmm v16, v17, v18, v19, v20, v21, v22, v23;
    Xbyak::Zmm v24, v25, v26, v27, v28, v29, v30, v31;

    RegMap()
        :name("m512"),
         v4(4), v5(5), v6(6), v7(7),
         v8(8), v9(9), v10(10), v11(11), v12(12), v13(13), v14(14), v15(15),
         v16(16), v17(17), v18(18), v19(19), v20(20), v21(21), v22(22), v23(23),
         v24(24), v25(25), v26(26), v27(27), v28(28), v29(29), v30(30), v31(31)
        {}

    bool vec_reg() {
        return true;
    }

    void save(Xbyak::CodeGenerator *g, Xbyak::Ymm r, int off, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->vmovdqa(g->ptr [g->rsp + off], r);
            break;
        case OT_FP32:
            g->vmovaps(g->ptr [g->rsp + off], r);
            break;
        case OT_FP64:
            g->vmovapd(g->ptr [g->rsp + off], r);
            break;
        }
    }

    void restore(Xbyak::CodeGenerator *g, Xbyak::Ymm r, int off, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->vmovdqa(r, g->ptr [g->rsp + off]);
            break;
        case OT_FP32:
            g->vmovaps(r, g->ptr [g->rsp + off]);
            break;
        case OT_FP64:
            g->vmovapd(r, g->ptr [g->rsp + off]);
            break;
        }
    }

    void killdep(Xbyak::CodeGenerator *g, Xbyak::Ymm r, enum operand_type ot) {
        switch (ot) {
        case OT_INT:
            g->vpxorq(r, r, r);
            break;
        case OT_FP32:
            g->vpxorq(r, r, r);
            break;
        case OT_FP64:
            g->vpxorq(r, r, r);
            break;
        }
    }
};



template <>
struct RegMap<Xbyak::Reg64>
{
    const char *name;
    Xbyak::Reg64 v4, v5, v6, v7;
    Xbyak::Reg64 v8, v9, v10, v11, v12, v13, v14, v15;

    RegMap()
        :name("reg64"),
         v4(Xbyak::Operand::RSP),
         v5(Xbyak::Operand::RBP),
         v6(Xbyak::Operand::RSI),
         v7(Xbyak::Operand::RDI),
         v8(Xbyak::Operand::R8),
         v9(Xbyak::Operand::R9),
         v10(Xbyak::Operand::R10),
         v11(Xbyak::Operand::R11),
         v12(Xbyak::Operand::R12),
         v13(Xbyak::Operand::R13),
         v14(Xbyak::Operand::R14),
         v15(Xbyak::Operand::R15)
        {}

    bool vec_reg() {
        return false;
    }

    void save(Xbyak::CodeGenerator *g, Xbyak::Reg64 r, int off, enum operand_type ) {
        g->mov(g->ptr[g->rsp + off], r);
    }

    void restore(Xbyak::CodeGenerator *g, Xbyak::Reg64 r, int off, enum operand_type ) {
        g->mov(r, g->ptr[g->rsp + off]);
    }

    void killdep(Xbyak::CodeGenerator *g, Xbyak::Reg64 r, enum operand_type) {
        g->xor_(r, r);
    }

};

template <typename RegType, typename Gen, typename F>
struct gen_throughput{
    void operator () (Gen *g, RegMap<RegType> &rm, F f, int num_insn){
        if (rm.vec_reg()) {
            for (int ii=0; ii<num_insn/12; ii++) {
                f(g, rm.v4, rm.v4);
                f(g, rm.v5, rm.v5);
                f(g, rm.v6, rm.v6);
                f(g, rm.v7, rm.v7);
                f(g, rm.v8, rm.v8);
                f(g, rm.v9, rm.v9);
                f(g, rm.v10, rm.v10);
                f(g, rm.v11, rm.v11);
                f(g, rm.v12, rm.v12);
                f(g, rm.v13, rm.v13);
                f(g, rm.v14, rm.v14);
                f(g, rm.v15, rm.v15);
            }
        } else {
            for (int ii=0; ii<num_insn/8; ii++) {
                f(g, rm.v8, rm.v8);
                f(g, rm.v9, rm.v9);
                f(g, rm.v10, rm.v10);
                f(g, rm.v11, rm.v11);
                f(g, rm.v12, rm.v12);
                f(g, rm.v13, rm.v13);
                f(g, rm.v14, rm.v14);
                f(g, rm.v15, rm.v15);
            }
        }
    }
};


template <typename RegType,
          typename F>
struct Gen
    :public Xbyak::CodeGenerator
{
    Gen(F f, int num_loop, int num_insn, enum lt_op o, enum operand_type ot) {
        RegMap<RegType> rm;

        int reg_size = 64;
        int num_reg = 12;

        push(rbp);
        mov(rbp, rsp);
        and_(rsp, -(Xbyak::sint64)64);
        sub(rsp, reg_size * (num_reg + 1));

        if (rm.vec_reg()) {
            rm.save(this, rm.v4,  -reg_size*12, ot);
            rm.save(this, rm.v5,  -reg_size*11, ot);
            rm.save(this, rm.v6,  -reg_size*10, ot);
            rm.save(this, rm.v7,  -reg_size*9, ot);
        }

        rm.save(this, rm.v8,  -reg_size*8, ot);
        rm.save(this, rm.v9,  -reg_size*7, ot);
        rm.save(this, rm.v10, -reg_size*6, ot);
        rm.save(this, rm.v11, -reg_size*5, ot);
        rm.save(this, rm.v12, -reg_size*4, ot);
        rm.save(this, rm.v13, -reg_size*3, ot);
        rm.save(this, rm.v14, -reg_size*2, ot);
        rm.save(this, rm.v15, -reg_size*1, ot);

        if (rm.vec_reg()) {
            rm.killdep(this, rm.v4, ot);
            rm.killdep(this, rm.v5, ot);
            rm.killdep(this, rm.v6, ot);
            rm.killdep(this, rm.v7, ot);
        }

        rm.killdep(this, rm.v8, ot);
        rm.killdep(this, rm.v9, ot);
        rm.killdep(this, rm.v10, ot);
        rm.killdep(this, rm.v11, ot);
        rm.killdep(this, rm.v12, ot);
        rm.killdep(this, rm.v13, ot);
        rm.killdep(this, rm.v14, ot);
        rm.killdep(this, rm.v15, ot);

        mov(rcx, num_loop);
        mov(rdx, (intptr_t)zero_mem);
        mov(ptr[rsp], rdi);
        xor_(rdi, rdi);

        align(16);
        L("@@");

        switch (o) {
        case LT_LATENCY:
            for (int ii=0; ii<num_insn; ii++) {
                f(this, rm.v8, rm.v8);
            }
            break;

        case LT_THROUGHPUT:
            gen_throughput<RegType,Gen,F>()(this, rm, f, num_insn);
            break;

        case LT_THROUGHPUT_KILLDEP:
            if (rm.vec_reg()) {
                for (int ii=0; ii<num_insn/12; ii++) {
                    f(this, rm.v4, rm.v4);
                    f(this, rm.v5, rm.v5);
                    f(this, rm.v6, rm.v6);
                    f(this, rm.v7, rm.v7);
                    f(this, rm.v8, rm.v8);
                    f(this, rm.v9, rm.v9);
                    f(this, rm.v10, rm.v10);
                    f(this, rm.v11, rm.v11);
                    f(this, rm.v12, rm.v12);
                    f(this, rm.v13, rm.v13);
                    f(this, rm.v14, rm.v14);
                    f(this, rm.v15, rm.v15);
                }

                rm.killdep(this, rm.v4, ot);
                rm.killdep(this, rm.v5, ot);
                rm.killdep(this, rm.v6, ot);
                rm.killdep(this, rm.v7, ot);
                rm.killdep(this, rm.v8, ot);
                rm.killdep(this, rm.v9, ot);
                rm.killdep(this, rm.v10, ot);
                rm.killdep(this, rm.v11, ot);
                rm.killdep(this, rm.v12, ot);
                rm.killdep(this, rm.v13, ot);
                rm.killdep(this, rm.v14, ot);
                rm.killdep(this, rm.v15, ot);
            } else {
                for (int ii=0; ii<num_insn/8; ii++) {
                    f(this, rm.v8, rm.v8);
                    f(this, rm.v9, rm.v9);
                    f(this, rm.v10, rm.v10);
                    f(this, rm.v11, rm.v11);
                    f(this, rm.v12, rm.v12);
                    f(this, rm.v13, rm.v13);
                    f(this, rm.v14, rm.v14);
                    f(this, rm.v15, rm.v15);
                }

                rm.killdep(this, rm.v8, ot);
                rm.killdep(this, rm.v9, ot);
                rm.killdep(this, rm.v10, ot);
                rm.killdep(this, rm.v11, ot);
                rm.killdep(this, rm.v12, ot);
                rm.killdep(this, rm.v13, ot);
                rm.killdep(this, rm.v14, ot);
                rm.killdep(this, rm.v15, ot);
                
            }
            break;
        }

        dec(rcx);
        jnz("@b");

        mov(rdi, ptr[rsp]);
        if (rm.vec_reg()) {
            rm.restore(this, rm.v4,  -reg_size*12, ot);
            rm.restore(this, rm.v5,  -reg_size*11, ot);
            rm.restore(this, rm.v6, -reg_size*10, ot);
            rm.restore(this, rm.v7, -reg_size*9, ot);
        }

        rm.restore(this, rm.v8,  -reg_size*8, ot);
        rm.restore(this, rm.v9,  -reg_size*7, ot);
        rm.restore(this, rm.v10, -reg_size*6, ot);
        rm.restore(this, rm.v11, -reg_size*5, ot);
        rm.restore(this, rm.v12, -reg_size*4, ot);
        rm.restore(this, rm.v13, -reg_size*3, ot);
        rm.restore(this, rm.v14, -reg_size*2, ot);
        rm.restore(this, rm.v15, -reg_size*1, ot);

        mov(rsp, rbp);
        pop(rbp);
        ret();
            
        /*
         * latency:
         *
         *       mov rcx, count
         * loop:
         *       op reg, reg
         *       op reg, reg
         *       ...
         *       op reg, reg
         *       dec rcx
         *       jne loop
         *
         */

        /*
         * throughput
         *
         *       mov rcx, count
         * loop:
         *       op reg8, reg8
         *       op reg9, reg9
         *       ...
         *       op reg15, reg15
         *       op reg8, reg8
         *       op reg9, reg9
         *       ...
         *       op reg15, reg15
         *       ...
         *       if kill_dep {
         *       xor r8
         *       xor r9
         *       ...
         *       xor r15
         *       }
         *       dec rcx
         *       jne loop
         *
         */
        
    }
};
    

template <typename RegType> int get_num_insn(void) {
    RegMap<RegType> rm;
    if (rm.vec_reg()) {
        return 36;
    } else {
        return 64;
    }
 }

template <typename RegType, typename F>
void
lt(const char *name,
   const char *on,
   F f,
   int num_loop,
   enum lt_op o,
   enum operand_type ot)
{
    int num_insn = get_num_insn<RegType>();

    Gen<RegType,F> g(f, num_loop, num_insn, o, ot);
    typedef void (*func_t)(void);
    func_t exec = (func_t)g.getCode();

    if (1) {
        char *p = (char*)g.getCode();
        int sz = g.getSize();
        FILE *fp = fopen("out.bin", "wb");
        for (int i=0; i<sz; i++) {
            fputc(p[i], fp);
        }
        fclose(fp);
    }

    memset(zero_mem, 0, sizeof(zero_mem));
    memset(data_mem, ~0, sizeof(data_mem));
    exec();

    long long b = read_cycle();
    exec();
    long long e = read_cycle();

    fprintf(logs,
            "\"%s\",\"%s\",\"%s\",\"%e\",\"%e\"\n",
            RegMap<RegType>().name,
            name, on,
            (e-b)/(double)(num_insn * num_loop), 
            (num_insn * num_loop)/(double)(e-b));

    if (output_csv) {
        printf("\"%s\",\"%s\",\"%s\",\"%e\",\"%e\"\n",
               RegMap<RegType>().name,
               name, on,
               (e-b)/(double)(num_insn * num_loop), 
               (num_insn * num_loop)/(double)(e-b));
    } else {
        printf("%8s:%40s:%10s: CPI=%8.2f, IPC=%8.2f\n",
               RegMap<RegType>().name,
               name, on,
               (e-b)/(double)(num_insn * num_loop), 
               (num_insn * num_loop)/(double)(e-b));
    }

}           

#define NUM_LOOP (16384*8)

template <typename RegType, typename F>
void
run(const char *name, F f, bool kill_dep, enum operand_type ot)
{
    lt<RegType>(name, "latency", f, NUM_LOOP, LT_LATENCY, ot);
    if (kill_dep) {
        lt<RegType>(name, "throughput", f, NUM_LOOP, LT_THROUGHPUT_KILLDEP, ot);
    } else {
        lt<RegType>(name, "throughput", f, NUM_LOOP, LT_THROUGHPUT, ot);
    }
}

template <typename RegType, typename F_t, typename F_l>
void
run_latency(const char *name, F_t f_t, F_l f_l, bool kill_dep, enum operand_type ot)
{
    lt<RegType>(name, "latency", f_l, NUM_LOOP, LT_LATENCY, ot);
    if (kill_dep) {
        lt<RegType>(name, "throughput", f_t, NUM_LOOP, LT_THROUGHPUT_KILLDEP, ot);
    } else {
        lt<RegType>(name, "throughput", f_t, NUM_LOOP, LT_THROUGHPUT, ot);
    }
}

template <typename RegType, typename F_l>
void
run_latency_only(const char *name, F_l f_l, bool kill_dep, enum operand_type ot)
{
    lt<RegType>(name, "latency", f_l, NUM_LOOP, LT_LATENCY, ot);
}


template <typename RegType, typename F_t>
void
run_throghput_only(const char *name, F_t f_t,bool kill_dep, enum operand_type ot)
{
    if (kill_dep) {
        lt<RegType>(name, "throughput", f_t, NUM_LOOP, LT_THROUGHPUT_KILLDEP, ot);
    } else {
        lt<RegType>(name, "throughput", f_t, NUM_LOOP, LT_THROUGHPUT, ot);
    }
}

#define GEN(rt, name, expr, kd, ot)                                     \
    run<Xbyak::rt>(                                                     \
    name,                                                               \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr;},   \
    kd, ot);


#define GEN_latency(rt, name, expr_t, expr_l, kd, ot)                   \
    run_latency<Xbyak::rt>(                                             \
    name,                                                               \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr_t;}, \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr_l;}, \
    kd, ot);


#define GEN_latency_only(rt, name, expr_l, kd, ot)              \
    run_latency_only<Xbyak::rt>(                                             \
    name,                                                           \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr_l;}, \
    kd, ot);

#define GEN_throughput_only(rt, name, expr_t, kd, ot)                   \
    run_throghput_only<Xbyak::rt>(                                      \
    name,                                                               \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr_t;}, \
    kd, ot);
        

int
main(int argc, char **argv)
{
    if (argc >= 2) {
        if (strcmp(argv[1],"--csv") == 0) {
            output_csv = true;
        }
    }

    cycle_counter_init();

#ifdef _WIN32
#define x_cpuid(p,eax) __cpuid(p, eax)
    typedef int cpuid_t;
#else
#define x_cpuid(p,eax) __get_cpuid(eax, &(p)[0], &(p)[1], &(p)[2], &(p)[3]);
    typedef unsigned int cpuid_t;
#endif

#ifdef _WIN32
    std::string path = "logs/w32/";
#else
    std::string path = "logs/linux/";

#endif

    {
        cpuid_t data[4*3+1];
        char data_nospace[4*3*4+1];

        x_cpuid(data+4*0, 0x80000002);
        x_cpuid(data+4*1, 0x80000003);
        x_cpuid(data+4*2, 0x80000004);
        data[12] = 0;
        puts((char*)data);

        char *d0 = (char*)data;
        int out = 0;

        for (int i=0; i<4*3*4; i++) {
            if (d0[i] != ' ') {
                data_nospace[out++] = d0[i];
            }
        }
        data_nospace[out] = '\0';

        path += data_nospace;
        path += ".csv";
    }

    logs = fopen(path.c_str(), "wb");
    fprintf(logs, 
            "class,inst,l/t,cpi,ipc\n");

    if (!output_csv) {
        printf("== latency/throughput ==\n");
    }
    GEN(Reg64, "add", (g->add(dst, src)), false, OT_INT);
    GEN(Reg64, "lea", (g->lea(dst, g->ptr[src])), false, OT_INT);
    GEN(Reg64, "xor dst,dst", (g->xor_(dst, dst)), false, OT_INT);
    GEN(Reg64, "xor", (g->xor_(dst, src)), false, OT_INT);
    GEN(Reg64, "load", (g->mov(dst, g->ptr[src + g->rdx])), false, OT_INT);
    GEN(Reg64, "crc32", (g->crc32(dst, src)), false, OT_INT);

    GEN(Reg64, "store [mem+0]->load[mem+0]", 
        (g->mov(g->ptr[src+g->rdx],g->rdi)) ; (g->mov(dst, g->ptr[g->rdx])),
        false, OT_INT);

    GEN(Reg64, "store [mem+0]->load[mem+1]", 
        (g->mov(g->ptr[src+g->rdx],g->rdi)) ; (g->mov(dst, g->ptr[g->rdx + 1])),
        false, OT_INT);

    GEN(Xmm, "pxor", (g->pxor(dst, src)), false, OT_INT);
    GEN(Xmm, "padd", (g->paddd(dst, src)), false, OT_INT);
    GEN(Xmm, "pmuldq", (g->pmuldq(dst, src)), false, OT_INT);

    /* 128 */
    GEN_throughput_only(Xmm, "loadps",
                        (g->movaps(dst, g->ptr[g->rdx])),
                        false, OT_INT);
    
    GEN_latency_only(Xmm, "loadps->movq",
                     (g->movaps(dst, g->ptr[g->rdx + g->rdi])); (g->movq(g->rdi, dst));,
                     false, OT_INT);

    GEN(Xmm, "movq->movq",
        (g->movq(g->rdi,src));(g->movq(dst,g->rdi));,
        false, OT_INT);

    GEN(Xmm, "xorps", (g->xorps(dst, src)), false, OT_FP32);
    GEN(Xmm, "addps", (g->addps(dst, src)), false, OT_FP32);
    GEN(Xmm, "mulps", (g->mulps(dst, src)), false, OT_FP32);
    GEN(Xmm, "divps", (g->divps(dst, src)), false, OT_FP32);
    GEN(Xmm, "divpd", (g->divpd(dst, src)), false, OT_FP64);
    GEN(Xmm, "rsqrtps", (g->rsqrtps(dst, dst)), false, OT_FP32);
    GEN(Xmm, "rcpps", (g->rcpps(dst, dst)), false, OT_FP32);
    GEN(Xmm, "blendps", (g->blendps(dst, src, 0)), false, OT_FP32);
    GEN_latency(Xmm, "blendvps",
                (g->blendvps(dst, src));(g->xorps(dst,dst)),
                (g->blendvps(dst, src)),
                false, OT_FP32);
    GEN(Xmm, "pshufb", (g->pshufb(dst, src)), false, OT_INT);
    GEN(Xmm, "shufps", (g->shufps(dst, src, 0)), false, OT_FP32);
    GEN(Xmm, "pmullw", (g->pmullw(dst, src)), false, OT_INT);
    GEN(Xmm, "phaddd", (g->phaddd(dst, src)), false, OT_INT);
    GEN(Xmm, "haddps", (g->phaddd(dst, src)), false, OT_FP32);

    GEN(Xmm, "pinsrd", 
        (g->pinsrb(dst, g->edx, 0)), false, OT_INT);
    GEN_latency_only(Xmm, "pinsrd->pexr", (g->pinsrb(dst, g->edx, 0));(g->pextrd(g->edx,dst,0)), false, OT_INT);
    GEN(Xmm, "dpps", (g->dpps(dst, src, 0xff)), false, OT_FP32);
    GEN(Xmm, "cvtps2dq", (g->cvtps2dq(dst, src)), false, OT_FP32);

    {
        bool have_avx = false;
        int reg[4];
        bool have_avx2 = false;
        bool have_fma = false;
        bool have_avx512f = false;
        bool have_avx512er = false;
        bool have_popcnt = false;
        bool have_aes = false;

#ifdef _WIN32
        __cpuidex(reg, 7, 0);
#else
        __cpuid_count(7, 0, reg[0], reg[1], reg[2], reg[3]);
#endif

        if (reg[1] & (1<<5)) {
            have_avx2 = true;
        }

        if (reg[1] & (1<<16)) {
            have_avx512f = true;
        }

        if (reg[1] & (1<<27)) {
            have_avx512er = true;
        }

#ifdef _WIN32
        __cpuid(reg, 1);
#else
        __cpuid(1, reg[0], reg[1], reg[2], reg[3]);
#endif
        if (reg[2] & (1<<12)) {
            have_fma = true;
        }

        if (reg[2] & (1<<28)) {
            have_avx = true;
        }

        if (reg[2] & (1<<23)) {
            have_popcnt = true;
        }

        if (reg[2] & (1<<25)) {
            have_aes = true;
        }


        if (have_popcnt) {
            GEN(Reg64, "popcnt", (g->popcnt(dst, src)), false, OT_INT);
        }

        if (have_aes) {
            GEN(Xmm, "aesenc", (g->aesenc(dst,src)), false, OT_INT);
            GEN(Xmm, "aesenclast", (g->aesenclast(dst,src)), false, OT_INT);
            GEN(Xmm, "aesdec", (g->aesdec(dst,src)), false, OT_INT);
            GEN(Xmm, "aesdeclast", (g->aesdeclast(dst,src)), false, OT_INT);
        }

        
        if (have_avx) {
            GEN_latency(Ymm, "movaps [mem]",
                        (g->vmovaps(dst, g->ptr[g->rdx])),
                        (g->vmovaps(dst, g->ptr[g->rdx + g->rdi])); (g->movq(g->rdi, dst)); ,
                        false, OT_FP32);

            GEN_latency(Ymm, "vmovdqu [mem+1]",
                        (g->vmovdqu(dst, g->ptr[g->rdx + 1])),
                        (g->vmovdqu(dst, g->ptr[g->rdx + g->rdi + 1])); (g->movq(g->rdi, dst)); ,
                        false, OT_FP32);

            GEN_latency(Ymm, "vmovdqu [mem+63] (cross cache)",
                        (g->vmovdqu(dst, g->ptr[g->rdx + 63])),
                        (g->vmovdqu(dst, g->ptr[g->rdx + g->rdi + 63])); (g->movq(g->rdi, dst)); ,
                        false, OT_FP32);

            GEN_latency(Ymm, "vmovdqu [mem+2MB-1] (cross page)",
                        (g->vmovdqu(dst, g->ptr[g->rdx + (2048*1024-1)])),
                        (g->vmovdqu(dst, g->ptr[g->rdx + g->rdi + (2048*1024-1)])); (g->movq(g->rdi, dst)); ,
                        false, OT_FP32);

            GEN(Ymm, "xorps", (g->vxorps(dst, dst, src)), false, OT_FP32);
            GEN(Ymm, "mulps", (g->vmulps(dst, dst, src)), false, OT_FP32);
            GEN(Ymm, "addps", (g->vaddps(dst, dst, src)), false, OT_FP32);
            GEN(Ymm, "divps", (g->vdivps(dst, dst, src)), false, OT_FP32);
            GEN(Ymm, "divpd", (g->vdivpd(dst, dst, src)), false, OT_FP64);
            GEN(Ymm, "rsqrtps", (g->vrsqrtps(dst, dst)), false, OT_FP32);
            GEN(Ymm, "rcpps", (g->vrcpps(dst, dst)), false, OT_FP32);
            GEN(Ymm, "sqrtps", (g->vsqrtps(dst, dst)), false, OT_FP32);
            GEN(Ymm, "vperm2f128", (g->vperm2f128(dst,dst,src,0)), false, OT_FP32);
        }

        if (have_avx2) {
            GEN(Ymm, "pxor", (g->vpxor(dst, dst, src)), false, OT_INT);
            GEN(Ymm, "paddd", (g->vpaddd(dst, dst, src)), false, OT_INT);
            GEN(Ymm, "vpermps", (g->vpermps(dst, dst, src)), false, OT_FP32);
            GEN(Ymm, "vpermpd", (g->vpermpd(dst, dst, 0)), false, OT_FP64);

            GEN_latency(Ymm, "vpmovsxwd",
                        (g->vpmovsxwd(g->ymm1,g->xmm0)),
                        (g->vpmovsxwd(g->ymm0,g->xmm0)),
                        false, OT_INT);

            GEN_latency(Ymm, "vpgatherdd",
                        (g->vpgatherdd(g->ymm2, g->ptr[g->rdx + g->ymm0*1], g->ymm1)),
                        (g->vpgatherdd(g->ymm2, g->ptr[g->rdx + g->ymm0*1], g->ymm1)); (g->vmovdqa(g->ymm0,g->ymm2)),
                        false, OT_INT);

            GEN_latency(Ymm, "gather32(<ld+ins>x8 + perm)",

                        /* throughput */
                        (g->vmovd(g->xmm2, g->ptr[g->rdx]));
                        (g->vmovd(g->xmm3, g->ptr[g->rdx]));
                        (g->vpinsrd(g->xmm2, g->xmm2, g->ptr[g->rdx + 4], 0));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + 4], 0));
                        (g->vpinsrd(g->xmm2, g->xmm2, g->ptr[g->rdx + 8], 0));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + 8], 0));
                        (g->vpinsrd(g->xmm2, g->xmm2, g->ptr[g->rdx + 12], 0));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + 12], 0));
                        (g->vperm2i128(g->ymm2,g->ymm2,g->ymm3,0));,

                        /* latency */
                        (g->vmovd(g->xmm2, g->ptr[g->rdx + g->rdi]));
                        (g->vmovd(g->xmm3, g->ptr[g->rdx + g->rdi]));
                        (g->vpinsrd(g->xmm2, g->xmm2, g->ptr[g->rdx + g->rdi + 4], 0));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + g->rdi + 4], 0));
                        (g->vpinsrd(g->xmm2, g->xmm2, g->ptr[g->rdx + g->rdi + 8], 0));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + g->rdi + 8], 0));
                        (g->vpinsrd(g->xmm2, g->xmm2, g->ptr[g->rdx + g->rdi + 12], 0));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + g->rdi + 12], 0));
                        (g->vperm2i128(g->ymm2,g->ymm2,g->ymm3,0));
                        (g->vmovd(g->edi, g->xmm2));

                        ,false, OT_FP32);


            GEN_latency(Ymm, "vgatherdpd",
                        (g->vgatherdpd(g->ymm2, g->ptr[g->rdx + g->xmm0*1], g->ymm1)),
                        (g->vgatherdpd(g->ymm2, g->ptr[g->rdx + g->xmm0*1], g->ymm1)); (g->vmovdqa(g->ymm0,g->ymm2)),
                        false, OT_INT);

            GEN_latency(Ymm, "gather64(<ld+ins>x4 + perm)",

                        /* throughput */
                        (g->vmovq(g->xmm2, g->ptr[g->rdx]));
                        (g->vmovq(g->xmm3, g->ptr[g->rdx]));
                        (g->vpinsrq(g->xmm2, g->xmm2, g->ptr[g->rdx + 8], 1));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + 8], 1));
                        (g->vperm2i128(g->ymm2,g->ymm2,g->ymm3,0));,

                        /* latency */
                        (g->vmovq(g->xmm2, g->ptr[g->rdx + g->rdi]));
                        (g->vmovq(g->xmm3, g->ptr[g->rdx + g->rdi]));
                        (g->vpinsrq(g->xmm2, g->xmm2, g->ptr[g->rdx + 8], 1));
                        (g->vpinsrd(g->xmm3, g->xmm3, g->ptr[g->rdx + 8], 1));
                        (g->vperm2i128(g->ymm2,g->ymm2,g->ymm3,0));
                        (g->vmovd(g->edi, g->xmm2));,

                        false, OT_FP32);

            GEN(Ymm, "vpshufb", (g->vpshufb(dst, src, src)), false, OT_INT);
        }

        if (have_fma) {
            GEN(Ymm, "vfmaps", (g->vfmadd132ps(dst, src, src)), false, OT_FP32);
            GEN(Ymm, "vfmapd", (g->vfmadd132pd(dst, src, src)), false, OT_FP64);
            GEN(Xmm, "vfmaps", (g->vfmadd132ps(dst, src, src)), false, OT_FP32);
            GEN(Xmm, "vfmapd", (g->vfmadd132pd(dst, src, src)), false, OT_FP64);
        }


        if (have_avx512f) {
            GEN(Zmm, "vfmaps", (g->vfmadd132ps(dst, src, src)), false, OT_FP32);
            GEN(Zmm, "vfmapd", (g->vfmadd132pd(dst, src, src)), false, OT_FP64);
            GEN(Zmm, "vfmaps reg, reg, [mem]", (g->vfmadd132pd(dst, src, g->ptr[g->rdx])), false, OT_FP32);
            GEN(Zmm, "vpexpandd", (g->vpexpandd(dst, src)), false, OT_FP32);
            GEN(Zmm, "vplzcntq", (g->vpexpandd(dst, src)), false, OT_FP32);
            GEN(Zmm, "vpconflictd", (g->vpconflictd(dst, src)), false, OT_FP32);
            GEN(Zmm, "vpermt2d", (g->vpermt2d(dst, src, src)), false, OT_FP32);
            GEN(Zmm, "vshufps", (g->vshufps(dst, src, src, 0)), false, OT_FP32);
            GEN(Zmm, "vrcp14pd", (g->vrcp14pd(dst, src)), false, OT_FP32);
            GEN(Zmm, "vpternlogd", (g->vpternlogd(dst, src, src, 0)), false, OT_FP32);
        }

        if (have_avx512er) {
            GEN(Zmm, "vrcp28pd", (g->vrcp28pd(dst, src)), false, OT_FP32);
        }
    }

#ifdef ENABLE_MPX
    /* MPX */
    GEN_throughput_only(Reg64, "bndcu",
                        (g->bndcu(g->bnd0, g->rax)),
                        false, OT_INT);
    GEN_throughput_only(Reg64, "bndmk",
                        (g->bndmk(g->bnd0, g->ptr[g->rax*4+100])),
                        false, OT_INT);
    GEN_throughput_only(Reg64, "bndmov(st)",
                        (g->bndmov(g->ptr[data_mem], g->bnd0)),
                        false, OT_INT);
    GEN_throughput_only(Reg64, "bndmov(ld)",
                        (g->bndmov(g->bnd0, g->ptr[data_mem])),
                        false, OT_INT);
    GEN_throughput_only(Reg64, "bndldx",
                        (g->bndldx(g->bnd0, g->ptr[g->rdx])),
                        false, OT_INT);
    GEN_throughput_only(Reg64, "bndstx",
                        (g->bndstx(g->ptr[g->rdx], g->bnd0)),
                        false, OT_INT);
#endif

    fclose(logs);
}
