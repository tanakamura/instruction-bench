#include <xbyak.h>
#include <stdio.h>

char data[4096];

struct G
  :public Xbyak::CodeGenerator
{
  G() {
    vfmadd132ps(zmm0, zmm1, zmm2);
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
