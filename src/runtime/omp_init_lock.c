
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_lock_t lock;
   omp_init_lock(&lock);
   int data = 0;
#pragma omp parallel num_threads(16) shared(lock, data) default(none)
   {
      omp_set_lock(&lock);
      data++;
      omp_unset_lock(&lock);
   }
   omp_destroy_lock(&lock);
   printf("data = %d\n", data);
   return 0;
}