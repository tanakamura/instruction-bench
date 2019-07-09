#include "common.hpp"

void
test_avx()
{
    if (info.have_avx) {
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

        GEN(Ymm, "vxorps", (g->vxorps(dst, dst, src)), false, OT_FP32);
        GEN(Ymm, "vmulps", (g->vmulps(dst, dst, src)), false, OT_FP32);
        GEN(Ymm, "vaddps", (g->vaddps(dst, dst, src)), false, OT_FP32);
        GEN(Ymm, "vdivps", (g->vdivps(dst, dst, src)), false, OT_FP32);
        GEN(Ymm, "vdivpd", (g->vdivpd(dst, dst, src)), false, OT_FP64);
        GEN(Ymm, "vrsqrtps", (g->vrsqrtps(dst, dst)), false, OT_FP32);
        GEN(Ymm, "vrcpps", (g->vrcpps(dst, dst)), false, OT_FP32);
        GEN(Ymm, "vsqrtps", (g->vsqrtps(dst, dst)), false, OT_FP32);
        GEN(Ymm, "vperm2f128", (g->vperm2f128(dst,dst,src,0)), false, OT_FP32);
    }

    if (info.have_avx2) {
        GEN(Ymm, "vpxor", (g->vpxor(dst, dst, src)), false, OT_INT);
        GEN(Ymm, "vpaddd", (g->vpaddd(dst, dst, src)), false, OT_INT);
        GEN(Ymm, "vpermps", (g->vpermps(dst, dst, src)), false, OT_FP32);
        GEN(Ymm, "vpermpd", (g->vpermpd(dst, dst, 0)), false, OT_FP64);
        GEN(Ymm, "vpblendvb", (g->vpblendvb(dst, src, src, src)), false, OT_INT);
        GEN_throughput_only(Ymm, "vpmovmskb", (g->vpmovmskb(g->edx,g->ymm0)), false, OT_INT);


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

    if (info.have_fma) {
        GEN(Ymm, "vfmaps", (g->vfmadd132ps(dst, src, src)), false, OT_FP32);
        GEN(Ymm, "vfmapd", (g->vfmadd132pd(dst, src, src)), false, OT_FP64);
        GEN(Xmm, "vfmaps", (g->vfmadd132ps(dst, src, src)), false, OT_FP32);
        GEN(Xmm, "vfmapd", (g->vfmadd132pd(dst, src, src)), false, OT_FP64);
    }
}