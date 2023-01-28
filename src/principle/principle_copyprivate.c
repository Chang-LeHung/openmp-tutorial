
#include <stdio.h>
#include <omp.h>

int x = 100;
#pragma omp threadprivate(x)

int main()
{
#pragma omp parallel num_threads(4) default(none) copyin(x)
  {
    x = omp_get_thread_num();
    printf("tid = %d x = %d\n", omp_get_thread_num(), x);
#pragma omp single copyprivate(x)
    {
      x = 200;
    }
    printf("tid = %d x = %d\n", omp_get_thread_num(), x);
  }
  return 0;
}