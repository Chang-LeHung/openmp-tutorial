
#include <stdio.h>
#include <omp.h>

int main()
{
   omp_lock_t lock;
   // 对锁进行初始化操作
   omp_init_lock(&lock);
   int data = 0;
#pragma omp parallel num_threads(16) shared(lock, data) default(none)
   {
      // 进行加锁处理 同一个时刻只能够有一个线程能够获取锁
      omp_set_lock(&lock);
      data++;
      // 解锁处理 线程在出临界区之前需要解锁 好让其他线程能够进入临界区
      omp_unset_lock(&lock);
   }
   omp_destroy_lock(&lock);
   printf("data = %d\n", data);
   return 0;
}