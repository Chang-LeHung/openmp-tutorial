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
➜  cmake-build-hun git:(master) ✗ export OMP_DISPLAY_ENV=TRUE   
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
OPENMP DISPLAY ENVIRONMENT END
data = 0
```

- OMP_DYNAMIC，如果将这个环境变量设置为true，OpenMP实现可以调整用于执行并行区域的线程数，以优化系统资源的使用。与这个环境变量相关的一共有两个函数：

```c
void omp_set_dynamic(int);
int omp_get_dynamic(void);
```

omp_set_dynamic 使用这个函数表示是否设置动态调整线程的个数，如果传入的参数不等于 0 表示开始，如果参数等于 0 就表示关闭动态调整。

我们现在来谈一谈 dynamic 动态调整线程个数以优化系统资源的使用是什么意思，这个意思就是 OpenMP 创建的线程个数在同一个时刻不会超过你系统的处理器的个数，因为 OpenMP 常常用在数据密集型任务当中，这类任务对 CPU 的需求大，因此为来充分利用资源，只会创建处理器个数的线程个数。

下面我们使用一个例子来验证上面锁谈到的内容。

```c
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
```

上面的代码如果我们没有设置 OMP_DYNAMIC=TRUE 或者没有使用 omp_set_dynamic(1) 去启动态调整的话，那么上面的 printf 语句会被执行 33 次，但是如果你进行了设置，也就是启动了动态调整线程的个数的话，那么创建的线程个数就是 min(33, num_processors) ，后者是你的机器的处理器的个数，比如如果处理器的核的个数是 16 那么就只会有 16 个线程执行并行域当中的代码。

- OMP_NESTED，这个表示是否开启并行域的嵌套模式，这个环境变量要么是 `TRUE` 或者 `FALSE` ，如果这个环境变量的值为 `TRUE` 那么能够嵌套的最大的并行域的数量受到环境变量 OMP_MAX_ACTIVE_LEVELS 的限制，与这个环境变量相关的一个动态库函数为 `void omp_set_nested(int nested);` ，表示是否开启嵌套的并行动态库。
- OMP_NUM_THREADS，这个表示设置并行域的线程个数，与这个环境变量相关的有 num_threads 这个子句和动态库函数 `void omp_set_num_threads(int num_threads);`也是相关的。他们的优先级为：num_threads > omp_set_num_threads > OMP_NUM_THREADS。这个环境变量的值必须是一个大于 0 的整数，关于他们的优先级你可以认为离并行域越远的就优先级越低，反之越高。
