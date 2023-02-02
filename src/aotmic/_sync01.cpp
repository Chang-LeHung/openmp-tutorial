

#include <stdio.h>

int main()
{
  int a = 1;
  int b = 2;
  int c = 3;
  c = __sync_fetch_and_add(&a, b);
  printf("a = %d b = %d c = %d\n", a, b, c);
  printf("%ld\n", sizeof (long));
  return 0;
}