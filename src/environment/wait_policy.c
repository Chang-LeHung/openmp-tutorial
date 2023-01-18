
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_lock_t lock;
   omp_init_lock(&lock);
#pragma omp parallel num_threads(16) default(none) shared(lock)
   {
      omp_set_lock(&lock);
      while (1);
      omp_unset_lock(&lock);
   }
   return 0;
}