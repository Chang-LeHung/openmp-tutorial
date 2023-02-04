
#include <stdio.h>


int main()
{
  int *intptr=0xff;
  int *ptrlock = 0xf;
  __asm__ volatile ("" : "=r" (intptr) : "0" (ptrlock));

  printf("%p %p\n", intptr, ptrlock);
  return 0;
}