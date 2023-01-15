
#include <stdio.h>
#include <omp.h>

void echo(int n, omp_lock_t * lock, int * s)
{
   if (n > 5)
   {
      omp_set_lock(lock);
      echo(n - 1, lock, s);
      *s += 1;
      omp_unset_lock(lock);
   }
   else
   {
      omp_set_lock(lock);
      *s += n;
      omp_unset_lock(lock);
   }
}

int main()
{
   int n = 100;
   int s = 0;
   omp_lock_t lock;
   omp_init_lock(&lock);
   echo(n, &lock, &s);
   printf("s = %d\n", s);
   omp_destroy_lock(&lock);
   return 0;
}