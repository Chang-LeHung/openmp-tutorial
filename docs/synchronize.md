# 深入理解 OpenMP 同步机制
## 前言
在本篇文章当中主要给大家介绍 OpenMP 当中线程的同步和互斥机制，在 OpenMP 当中主要有三种不同的线程之间的互斥方式：

- 使用 critical 子句，使用这个子句主要是用于创建临界区和 OpenMP 提供的运行时库函数的作用是一致的，只不过这种方法是直接通过编译指导语句实现的，更加方便一点，加锁和解锁的过程编译器会帮我们实现。
- 使用 atomic 指令，这个主要是通过原子指令，主要是有处理器提供的一些原子指令实现的。
- OpenMP 给我们提供了 omp_lock_t 和 omp_nest_lock_t 两种数据结构实现简单锁和可重入锁。

在本篇文章当中主要讨论 OpenMP 当中的互斥操作，在下一篇文章当中主要讨论 OpenMP 当中原子操作的实现原理，并且查看程序编译之后的汇编指令。

## OpenMP 中的事件同步机制

在这一小节当中主要分析 OpenMP 当中的一些构造语句中的同步关系—— single, sections, for ，并且消除这些指令造成的线程之间的同步。

### Sections 使用 nowait 

在 OpenMP 当中 sections 主要是使不同的线程同时执行不同的代码块，但是在每个 `#pragma omp sections ` 区域之后有一个隐藏的同步代码块，也就是说只有所有的 section 被执行完成之后，并且所有的线程都到达同步点，线程才能够继续执行，比如在下面的代码当中，`printf("tid = %d finish sections\n", omp_get_thread_num())` 语句只有前面的 sections 块全部被执行完成，所有的线程才会开始执行这条语句，根据这一点在上面的 printf 语句执行之前所有的 section 当中的语句都会被执行。

```c


#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
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

```

上面的代码其中的一种输出结果如下所示：

```shell
tid = 1 sleep 1 seconds
tid = 2 sleep 2 seconds
tid = 3 sleep 3 seconds
tid = 4 sleep 4 seconds
tid = 3 finish sections
tid = 2 finish sections
tid = 0 finish sections
tid = 1 finish sections
```

上面的输出结果是符合我们的预期的，所有的 section 中的 printf 语句打印在最后一个 printf前面，这是因为 sections 块之后又一个隐藏的同步点，只有所有的线程达到同步点之后程序才会继续往后执行。

从上面的分析来看，很多时候我们是不需要一个线程执行完成之后等待其他线程的，也就是说如果一个线程的 section 执行完成之后而且没有其他的 section 没有被执行，那么我们就不必让这个线程挂起继续执行后面的任务，在这种情况下我们就可以使用 nowait ，使用的编译指导语句是 `#pragma omp sections nowait` ，具体的代码如下所示：

```c


#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
   {
#pragma omp sections nowait
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
```

上面的程序的输出结果如下所示：

```shell
tid = 1 sleep 1 seconds
tid = 0 finish sections
tid = 2 sleep 2 seconds
tid = 1 finish sections
tid = 3 sleep 3 seconds
tid = 2 finish sections
tid = 4 sleep 4 seconds
tid = 3 finish sections
```

从上面的输出结果我们可以看到，当一个线程的 section 代码执行完成之后，这个线程就立即执行最后的 printf 语句了，也就是说执行完成之后并没有等待其他的线程，这就是我们想要的效果。

### Single 使用 nowait

在 OpenMP 当中使用 single 指令表示只有一个线程执行 single 当中的代码，但是需要了解的是在 single 代码块最后 OpenMP 也会帮我们生成一个隐藏的同步点，只有执行 single 代码块的线程执行完成之后，所有的线程才能够继续往后执行。比如下面的示例程序：

```c


#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   double start = omp_get_wtime();
#pragma omp parallel num_threads(4) default(none) shared(start)
   {
#pragma omp single
      sleep(5);
      printf("tid = %d spent %lf s\n", omp_get_thread_num(), omp_get_wtime() - start);
   }
   double end = omp_get_wtime();
   printf("execution time : %lf", end - start);
   return 0;
}
```

在上面的代码当中启动了 4 个线程，在 single 的代码块当中需要 sleep 5秒钟，因为上面的代码不带 nowait，因此虽然之后一个线程执行 sleep(5)，但是因为其他的线程需要等待这个线程执行完成，因此所有的线程都需要等待 5 秒。因此可以判断上面的代码输出就是每个线程输出的时间差都是 5 秒左右。具体的上面的代码执行结果如下所示：

```shell
tid = 2 spent 5.002628 s
tid = 3 spent 5.002631 s
tid = 0 spent 5.002628 s
tid = 1 spent 5.005032 s
execution time : 5.005076
```

从上面的输出结果来看正符合我们的预期，每个线程花费的时间都是 5 秒左右。

现在我们使用 nowait 那么当一个线程执行 single 代码块的时候，其他线程就不需要进行等待了，那么每个线程花费的时间就非常少。我们看下面的使用 nowait 的程序的输出结果：

```c


#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   double start = omp_get_wtime();
#pragma omp parallel num_threads(4) default(none) shared(start)
   {
#pragma omp single nowait
      sleep(5);
      printf("tid = %d spent %lf s\n", omp_get_thread_num(), omp_get_wtime() - start);
   }
   double end = omp_get_wtime();
   printf("execution time : %lf", end - start);
   return 0;
}
```

上面的代码执行结果如下所示：

```shell
tid = 2 spent 0.002375 s
tid = 0 spent 0.003188 s
tid = 1 spent 0.003202 s
tid = 3 spent 5.002462 s
execution time : 5.002538
```

可以看到的是线程 3 执行了 single 代码块但是其他的线程并没有执行，而我们也使用了 nowait  因此每个线程花费的时间会非常少，这也是符合我们的预期。

### For 使用 nowait

for 的原理其实和上面两个使用方式也是一样的，都是不需要在同步点进行同步，然后直接执行后面的代码。话不多说直接看代码

```c


#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   double start = omp_get_wtime();
#pragma omp parallel num_threads(4) default(none) shared(start)
   {
#pragma omp for
      for(int i = 0; i < 4; ++i)
      {
         sleep(i);
      }
      printf("tid = %d spent %lf s\n", omp_get_thread_num(), omp_get_wtime() - start);
   }
   double end = omp_get_wtime();
   printf("execution time : %lf", end - start);
   return 0;
}
```

在上面的程序当中启动的一个 for 循环，有四个线程去执行这个循环，按照默认的调度方式第 i 个线程对应的 i 的值就是等于 i 也就是说，最长的一个线程 sleep 的时间为 3 秒，但是  sleep 1 秒或者 2 秒和 3 秒的线程需要进行等待，因此上面的程序的输出结果大概都是 3 秒左右。具体的结果如下图所示：

```shell
tid = 0 spent 3.003546 s
tid = 1 spent 3.003549 s
tid = 2 spent 3.003558 s
tid = 3 spent 3.003584 s
execution time : 3.005994
```

现在如果我们使用 mowait 那么线程不需要进行等待，那么线程的话费时间大概是 0 秒 1 秒 2 秒 3 秒。

```c
#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main()
{
   double start = omp_get_wtime();
#pragma omp parallel num_threads(4) default(none) shared(start)
   {
#pragma omp for nowait
      for(int i = 0; i < 4; ++i)
      {
         sleep(i);
      }
      printf("tid = %d spent %lf s\n", omp_get_thread_num(), omp_get_wtime() - start);
   }
   double end = omp_get_wtime();
   printf("execution time : %lf", end - start);
   return 0;
}
```

查看下面的结果，也是符号我们的预期的，因为线程之间不需要进行等待了。

```shell
tid = 0 spent 0.002358 s
tid = 1 spent 1.004497 s
tid = 2 spent 2.002433 s
tid = 3 spent 3.002427 s
execution time : 3.002494
```

