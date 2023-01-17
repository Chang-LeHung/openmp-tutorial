# OpenMP 环境变量使用总结

- OMP_CANCELLATION，在 OpenMP 规范 4.5 当中规定了取消机制，我们可以使用这个环境变量去设置是否启动取消机制，如果这个值等于 TRUE 那么就是开启线程取消机制，如果这个值等于 FALSE 那么就是关闭取消机制。

```c

#include <stdio.h>
#include <omp.h>

int main()
{

   int s = omp_get_cancellation();
   printf("%d\n", s);
#pragma omp parallel num_threads(8) default(none)
   {
      if (omp_get_thread_num() == 2)
      {
#pragma omp cancel parallel
      }

      printf("tid = %d\n", omp_get_thread_num());

   }
   return 0;
}
```

在上面的程序当中，如果我们启动取消机制，那么线程号等于 2 的线程就不会执行后面的 printf 语句。

```shell
➜  cmake-build-hun git:(master) ✗  export OMP_CANCELLATION=TRUE # 启动取消机制
➜  cmake-build-hun git:(master) ✗ ./cancel 
1
tid = 0
tid = 4
tid = 1
tid = 3
tid = 5
tid = 6
tid = 7
```

- OMP_DISPLAY_ENV，这个环境变量的作用就是程序在执行的时候首先会打印 OpenMP 相关的环境变量。如何这个环境变量值等于 TRUE 就会打印环境变量的值，如果是 FLASE 就不会打印。

```shell
➜  cmake-build-hun git:(master) ✗ export OMP_DISPLAY_ENV=VERBOSE
➜  cmake-build-hun git:(master) ✗ ./critical                    

OPENMP DISPLAY ENVIRONMENT BEGIN
  _OPENMP = '201511'
  OMP_DYNAMIC = 'FALSE'
  OMP_NESTED = 'FALSE'
  OMP_NUM_THREADS = '32'
  OMP_SCHEDULE = 'DYNAMIC'
  OMP_PROC_BIND = 'FALSE'
  OMP_PLACES = ''
  OMP_STACKSIZE = '0'
  OMP_WAIT_POLICY = 'PASSIVE'
  OMP_THREAD_LIMIT = '4294967295'
  OMP_MAX_ACTIVE_LEVELS = '2147483647'
  OMP_CANCELLATION = 'TRUE'
  OMP_DEFAULT_DEVICE = '0'
  OMP_MAX_TASK_PRIORITY = '0'
  OMP_DISPLAY_AFFINITY = 'FALSE'
  OMP_AFFINITY_FORMAT = 'level %L thread %i affinity %A'
  GOMP_CPU_AFFINITY = ''
  GOMP_STACKSIZE = '0'
  GOMP_SPINCOUNT = '300000'
OPENMP DISPLAY ENVIRONMENT END
data = 0
```

