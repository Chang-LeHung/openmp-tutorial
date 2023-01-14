
#include <stdio.h>
#include <omp.h>

int main()
{
   int nprocs = omp_get_num_procs();
   printf("Number of processors = %d\n", nprocs);
   return 0;
}