#include <xbyak.h>
#include <stdio.h>

struct G
  :public Xbyak::CodeGenerator
{
  G() {
    vpaddd(ymm0, ymm1, ymm2);
    vpxor(ymm0, ymm1, ymm2);
    vpermd(ymm0, ymm1, ymm2);
    vpermps(ymm0, ymm1, ymm2);
    vpermpd(ymm0, ymm1, 0);
  }
};

int main()
{
  G g;
  char *p = (char*)g.getCode();
  int sz = g.getSize();
  FILE *fp = fopen("out.bin", "wb");

  for (int i=0; i<sz; i++) {
    fputc(p[i], fp);
  }
}
