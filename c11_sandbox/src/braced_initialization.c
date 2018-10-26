
#include <stdio.h>

int main(int argc, char const *argv[])
{
  printf("sizeof char literal = %zu\n", sizeof 'S');
  int x=4;
  printf("sizeof int = %zu, address = %p\n", sizeof x, &x);
  char str4[] = {'S', 'U', 'N'};
  char str4real[] = "SUN";
  printf("str4: `%s`, size = %zu, address = %p\n\n", str4, sizeof str4, &str4[0]);
  printf("str4real: `%s`, size = %zu, address = %p\n\n", str4real, sizeof str4real, &str4real[0]);
  return 0;
}


