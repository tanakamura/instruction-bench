#include "common.hpp"

void
test_mpx()
{
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
}