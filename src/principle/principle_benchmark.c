

#include <stdio.h>
#include <omp.h>

void fetch_and_add(long size)
{
  long start = 0;
  for(long i = 0; i < size; ++i)
  {
    __sync_fetch_and_add(&start, 1);
  }
  printf("start = %ld\n", start);
}

void compare_and_swap(long size)
{
  long start = 0;
  for(long i = 0; i < size; ++i)
  {
    __sync_val_compare_and_swap(&start, start, start + 1);
  }
  printf("start = %ld\n", start);
}

int main()
{
  long size = 1000;
  double s = omp_get_wtime();
  fetch_and_add(size);
  double e = omp_get_wtime();
  printf("fetch_and_add : cost time = %lf\n", e - s);
  s = omp_get_wtime();
  compare_and_swap(size);
  e = omp_get_wtime();
  printf("compare_and_swap : cost time = %lf\n", e - s);
  return 0;
}