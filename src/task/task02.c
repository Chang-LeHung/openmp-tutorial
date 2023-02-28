

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

void print_task(int i)
{
   int tid = omp_get_thread_num();
   int nthtreas = omp_get_num_threads();
   printf("task num = %d tid = %d nthread = %d\n", i, tid, nthtreas);
}


void foo(void)
{
   for(int i = 0; i < 3; ++i)
   {
#pragma omp task firstprivate(i) default(none)
      print_task(i);
   }
}

int main()
{

   foo(); // orphaned task
   printf("split line\n");
   sleep(1);
#pragma omp parallel num_threads(4) default(none)
   {
#pragma omp single
      foo();
   }
   return 0;
}