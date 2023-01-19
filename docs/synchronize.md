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

从上面的输出结果我们可以看到，当一个线程的 section 代码执行完成之后