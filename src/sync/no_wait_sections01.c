

#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main() {
#pragma omp parallel num_threads(4) default(none)
   {
#pragma omp sections
      {
#pragma omp section
         {
            int s = omp_get_thread_num() + 1;
            sleep(s);
            printf("tid = %d sleep %d seconds\n", s, s);
         }
#pragma omp section
         {
            int s = omp_get_thread_num() + 1;
            sleep(s);
            printf("tid = %d sleep %d seconds\n", s, s);
         }
#pragma omp section
         {
            int s = omp_get_thread_num() + 1;
            sleep(s);
            printf("tid = %d sleep %d seconds\n", s, s);
         }
#pragma omp section
         {
            int s = omp_get_thread_num() + 1;
            sleep(s);
            printf("tid = %d sleep %d seconds\n", s, s);
         }
      }

      printf("tid = %d finish sections\n", omp_get_thread_num());
   }
   return 0;
}

