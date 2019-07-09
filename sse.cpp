#include "common.hpp"

void test_sse()
{
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
    GEN_latency_only(Xmm, "pinsrd->pextr", (g->pinsrb(dst, g->edx, 0));(g->pextrd(g->edx,dst,0)), false, OT_INT);
    GEN(Xmm, "dpps", (g->dpps(dst, src, 0xff)), false, OT_FP32);
    GEN(Xmm, "cvtps2dq", (g->cvtps2dq(dst, src)), false, OT_FP32);

    GEN_latency(Xmm, "movaps [mem]",
                (g->movaps(dst, g->ptr[g->rdx])),
                (g->movaps(dst, g->ptr[g->rdx + g->rdi])); (g->movq(g->rdi, dst)); ,
                false, OT_FP32);

    GEN_latency(Xmm, "movdqu [mem+1]",
                (g->movdqu(dst, g->ptr[g->rdx + 1])),
                (g->movdqu(dst, g->ptr[g->rdx + g->rdi + 1])); (g->movq(g->rdi, dst)); ,
                false, OT_FP32);

    GEN_latency(Xmm, "movdqu [mem+63] (cross cache)",
                (g->movdqu(dst, g->ptr[g->rdx + 63])),
                (g->movdqu(dst, g->ptr[g->rdx + g->rdi + 63])); (g->movq(g->rdi, dst)); ,
                false, OT_FP32);

    GEN_latency(Xmm, "movdqu [mem+2MB-1] (cross page)",
                (g->movdqu(dst, g->ptr[g->rdx + (2048*1024-1)])),
                (g->movdqu(dst, g->ptr[g->rdx + g->rdi + (2048*1024-1)])); (g->movq(g->rdi, dst)); ,
                false, OT_FP32);
}