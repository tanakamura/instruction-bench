#include <immintrin.h>
#include <x86intrin.h>
#include <cpuid.h>
#include <string.h>
#include "common.hpp"

bool output_csv = false;
FILE *logs;
cpuinfo info;

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

#else

#define cycle_counter_init() ((void)0)

#endif

char MIE_ALIGN(2048*1024) zero_mem[4096*1024];
char MIE_ALIGN(2048*1024) data_mem[4096*1024];

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
    if (logs == NULL) {
        perror(path.c_str());
        return 1;
    }
    fprintf(logs, 
            "class,inst,l/t,cpi,ipc\n");

    if (!output_csv) {
        printf("== latency/throughput ==\n");
    }

    {
        int reg[4];

#ifdef _WIN32
        __cpuidex(reg, 7, 0);
#else
        __cpuid_count(7, 0, reg[0], reg[1], reg[2], reg[3]);
#endif

        if (reg[1] & (1<<5)) {
            info.have_avx2 = true;
        }

        if (reg[1] & (1<<16)) {
            info.have_avx512f = true;
        }

        if (reg[1] & (1<<27)) {
            info.have_avx512er = true;
        }

#ifdef _WIN32
        __cpuid(reg, 1);
#else
        __cpuid(1, reg[0], reg[1], reg[2], reg[3]);
#endif
        if (reg[2] & (1<<1)) {
            info.have_pclmulqdq = true;
        }

        if (reg[2] & (1<<12)) {
            info.have_fma = true;
        }


        if (reg[2] & (1<<20)) {
            info.have_sse42 = true;
        }

        if (reg[2] & (1<<28)) {
            info.have_avx = true;
        }

        if (reg[2] & (1<<23)) {
            info.have_popcnt = true;
        }

        if (reg[2] & (1<<25)) {
            info.have_aes = true;
        }

        if (reg[2] & (1<<11)) {
            info.have_avx512vnni = true;
        }

#ifdef _WIN32
        __cpuidex(reg, 7, 1);
#else
        __cpuid_count(7, 1, reg[0], reg[1], reg[2], reg[3]);
#endif

        if (reg[0] & (1<<5)) {
            info.have_avx512bf16 = true;
        }

    }

    test_generic();
    test_sse();
    test_avx();
    test_avx512();

    if (info.have_popcnt) {
        GEN(Reg64, "popcnt", (g->popcnt(dst, src)), false, OT_INT);
    }

    if (info.have_aes) {
        GEN(Xmm, "aesenc", (g->aesenc(dst,src)), false, OT_INT);
        GEN(Xmm, "aesenclast", (g->aesenclast(dst,src)), false, OT_INT);
        GEN(Xmm, "aesdec", (g->aesdec(dst,src)), false, OT_INT);
        GEN(Xmm, "aesdeclast", (g->aesdeclast(dst,src)), false, OT_INT);
    }

    if (info.have_pclmulqdq) {
        GEN(Xmm, "pclmulqdq", (g->pclmulqdq(dst,src,0)), false, OT_INT);
    }

    fclose(logs);
}
