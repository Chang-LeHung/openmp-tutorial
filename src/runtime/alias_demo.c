
#include <stdio.h>

void __f () {
   printf("Hello __f\n");
}

void f () __attribute__ ((strong, alias ("__f")));

int main()
{
   f();
   __f();
   return 0;
}