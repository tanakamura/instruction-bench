#include <xbyak.h>
#include <stdio.h>

char data[4096];

struct G
  :public Xbyak::CodeGenerator
{
  G() {
    bndmk(bnd0, ptr[r10*4+4]);
    bndmov(bnd0, ptr[r10*4+4]);
    bndmov(ptr[r10*4+4], bnd0);
    bndldx(bnd0, ptr[r10*4+4]);
    bndstx(ptr[r10*4+4], bnd0);
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
