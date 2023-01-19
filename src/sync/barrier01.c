

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   omp_set_nested(1);
#pragma omp parallel num_threads(4) default(none)
   {
      int parent_id = omp_get_thread_num();
      printf("tid = %d\n", parent_id);
      sleep(1);
#pragma omp barrier
#pragma omp parallel num_threads(4) shared(parent_id) default(none)
      {
         sleep((parent_id + 1) * 2);
         printf("parent_id = %d tid = %d\n", parent_id, omp_get_thread_num());
#pragma omp barrier
         sleep(1);
         printf("after barrier : parent_id = %d tid = %d\n", parent_id, omp_get_thread_num());
      }
   }
   return 0;
}