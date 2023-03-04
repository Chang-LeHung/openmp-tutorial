

#include <stdio.h>
#include <omp.h>

int main()
{

#pragma omp parallel num_threads(4) default(none)
  {
     int data = omp_get_thread_num();
#pragma omp task default(none) firstprivate(data) if(data > 100)
    {
       printf("data = %d Hello World from tid = %d\n", data, omp_get_thread_num());
    }
  }
  return 0;
}