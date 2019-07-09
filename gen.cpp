#include "common.hpp"

void test_generic()
{
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
}