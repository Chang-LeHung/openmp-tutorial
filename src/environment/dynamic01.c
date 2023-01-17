#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
//   omp_set_dynamic(1);

#pragma omp parallel num_threads(33) default(none)
   {
      printf("tid = %d\n", omp_get_thread_num());
   }

   return 0;
}