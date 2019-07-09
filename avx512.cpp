#include "common.hpp"

void test_avx512() {
    if (info.have_avx512f) {
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

    if (info.have_avx512er) {
        GEN(Zmm, "vrcp28pd", (g->vrcp28pd(dst, src)), false, OT_FP32);
    }
}