

#include <stdio.h>
#include <omp.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>

int main()
{
#pragma omp parallel for ordered num_threads(4) default(none)
  for(int i = 0; i < 8; ++i) {
#pragma omp ordered
    {
      printf("tid = %d i = %d\n", omp_get_thread_num(), i);
    }
  }
  int data = (-INT32_MAX - 1);
  for(int i = 31; i >= 0; --i)
  {
    if((data >> i) & 1)
      printf("1");
    else
      printf("0");
  }
  printf("\n");
  return 0;
}