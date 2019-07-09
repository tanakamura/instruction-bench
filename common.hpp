#ifndef COMMON_HPP
#define COMMON_HPP

#include <xbyak.h>
#include <string.h>

struct cpuinfo {
    bool have_sse42 = false;
    bool have_avx = false;
    bool have_avx2 = false;
    bool have_fma = false;
    bool have_avx512f = false;
    bool have_avx512er = false;
    bool have_popcnt = false;
    bool have_aes = false;
    bool have_pclmulqdq = false;
};

extern cpuinfo info;
extern bool output_csv;
extern FILE *logs;
extern int perf_fd;

#ifdef __linux

#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <sys/eventfd.h>

static inline long long
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

#define read_cycle() __rdtsc()


#endif



extern char MIE_ALIGN(2048*1024) zero_mem[4096*1024];
extern char MIE_ALIGN(2048*1024) data_mem[4096*1024];

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
    Gen(F f, bool reserve_rcx, int num_loop, int num_insn, enum lt_op o, enum operand_type ot) {
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

        Xbyak::Reg64 counter_reg = rcx;
        if (reserve_rcx) {
            counter_reg = rdx;
            mov(rcx, 16);
            mov(rax, 16);
        } else {
            mov(rdx, (intptr_t)zero_mem);
        }

        mov(counter_reg, num_loop);
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

        dec(counter_reg);
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
   bool reserve_rcx,
   int num_loop,
   enum lt_op o,
   enum operand_type ot)
{
    int num_insn = get_num_insn<RegType>();

    Gen<RegType,F> g(f, reserve_rcx, num_loop, num_insn, o, ot);
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
    lt<RegType>(name, "latency", f, false, NUM_LOOP, LT_LATENCY, ot);
    if (kill_dep) {
        lt<RegType>(name, "throughput", f, false, NUM_LOOP, LT_THROUGHPUT_KILLDEP, ot);
    } else {
        lt<RegType>(name, "throughput", f, false, NUM_LOOP, LT_THROUGHPUT, ot);
    }
}

template <typename RegType, typename F_t, typename F_l>
void
run_latency(const char *name, F_t f_t, F_l f_l, bool kill_dep, enum operand_type ot)
{
    lt<RegType>(name, "latency", f_l, false, NUM_LOOP, LT_LATENCY, ot);
    if (kill_dep) {
        lt<RegType>(name, "throughput", f_t, false, NUM_LOOP, LT_THROUGHPUT_KILLDEP, ot);
    } else {
        lt<RegType>(name, "throughput", f_t, false, NUM_LOOP, LT_THROUGHPUT, ot);
    }
}

template <typename RegType, typename F_l>
void
run_latency_only(const char *name, F_l f_l, bool kill_dep, bool reserve_rcx, enum operand_type ot)
{
    lt<RegType>(name, "latency", f_l, reserve_rcx, NUM_LOOP, LT_LATENCY, ot);
}


template <typename RegType, typename F_t>
void
run_throghput_only(const char *name, F_t f_t,bool kill_dep, bool reserve_rcx, enum operand_type ot)
{
    if (kill_dep) {
        lt<RegType>(name, "throughput", f_t, reserve_rcx, NUM_LOOP, LT_THROUGHPUT_KILLDEP, ot);
    } else {
        lt<RegType>(name, "throughput", f_t, reserve_rcx, NUM_LOOP, LT_THROUGHPUT, ot);
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
    kd, false, ot);

#define GEN_latency_only_rcx_clobber(rt, name, expr_l, kd, ot)          \
    run_latency_only<Xbyak::rt>(                                        \
    name,                                                           \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr_l;}, \
    kd, true, ot);

#define GEN_throughput_only(rt, name, expr_t, kd, ot)                   \
    run_throghput_only<Xbyak::rt>(                                      \
    name,                                                               \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr_t;}, \
    kd, false, ot);

#define GEN_throughput_only_rcx_clobber(rt, name, expr_t, kd, ot)       \
    run_throghput_only<Xbyak::rt>(                                      \
    name,                                                               \
    [](Xbyak::CodeGenerator *g, Xbyak::rt dst, Xbyak::rt src){expr_t;}, \
    kd, true, ot);
        

extern void test_generic();
extern void test_mpx();
extern void test_avx512();
extern void test_avx();
extern void test_sse();

#endif