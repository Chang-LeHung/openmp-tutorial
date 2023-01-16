
#include <stdio.h>
#include <omp.h>

void echo(int n, omp_nest_lock_t* lock, int * s)
{
   if (n > 5)
   {
      omp_set_nest_lock(lock);
      // 在这里进行递归调用 因为在上一行代码已经获取锁了 递归调用还需要获取锁
      // omp_lock_t 是不能满足这个要求的 而 omp_nest_lock_t 能
      echo(n - 1, lock, s);
      *s += 1;
      omp_unset_nest_lock(lock);
   }
   else
   {
      omp_set_nest_lock(lock);
      *s += n;
      omp_unset_nest_lock(lock);
   }
}

int main()
{
   int n = 100;
   int s = 0;
   omp_nest_lock_t lock;
   omp_init_nest_lock(&lock);
   echo(n, &lock, &s);
   printf("s = %d\n", s);
   omp_destroy_nest_lock(&lock);
   printf("%ld\n", sizeof (omp_nest_lock_t));
   return 0;
}