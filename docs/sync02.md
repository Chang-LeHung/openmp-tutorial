# OpenMP 线程同步 Construct 实现原理以及源码分析（下）

## 前言

在上面文章当中我们主要分析了 flush, critical, master 这三个 construct 的实现原理。在本篇文章当中我们将主要分析另外三个 construct : barrier 、ordered 和 single 。

## Barrier Construct

### 编译器角度分析

在本小节当中我们主要介绍 `#pragma omp barrier` 的使用，事实上这个 construct 在编译器的处理上非常简单，只是将这条编译指导语句变成了一个函数调用。

```c
void GOMP_barrier (void)
```

每一条 `#pragma omp barrier` 都会变成调用函数 GOMP_barrier 。我们来看一个示例程序：

```c


#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
    printf("tid = %d start\n", omp_get_thread_num());
#pragma omp barrier
    printf("tid = %d end\n", omp_get_thread_num());
  }
  return 0;
}
```

在前面的文章当中我们已经提到了编译器会将一个 parallel construct 编译成一个函数，上面的 parallel construct 被编译的之后的结果如下所示，可以看到确实编译成了调用函数 GOMP_barrier 。

```asm
000000000040118a <main._omp_fn.0>:
  40118a:       55                      push   %rbp
  40118b:       48 89 e5                mov    %rsp,%rbp
  40118e:       48 83 ec 10             sub    $0x10,%rsp
  401192:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  401196:       e8 a5 fe ff ff          callq  401040 <omp_get_thread_num@plt>
  40119b:       89 c6                   mov    %eax,%esi
  40119d:       bf 10 20 40 00          mov    $0x402010,%edi
  4011a2:       b8 00 00 00 00          mov    $0x0,%eax
  4011a7:       e8 a4 fe ff ff          callq  401050 <printf@plt>
  4011ac:       e8 7f fe ff ff          callq  401030 <GOMP_barrier@plt>
  4011b1:       e8 8a fe ff ff          callq  401040 <omp_get_thread_num@plt>
  4011b6:       89 c6                   mov    %eax,%esi
  4011b8:       bf 20 20 40 00          mov    $0x402020,%edi
  4011bd:       b8 00 00 00 00          mov    $0x0,%eax
  4011c2:       e8 89 fe ff ff          callq  401050 <printf@plt>
  4011c7:       c9                      leaveq 
  4011c8:       c3                      retq   
  4011c9:       0f 1f 80 00 00 00 00    nopl   0x0(%rax)
```

### 源码分析

```c
void
GOMP_barrier (void)
{
  // 得到当前线程的相关数据
  struct gomp_thread *thr = gomp_thread ();
  // 得到当前线程的线程组
  struct gomp_team *team = thr->ts.team;

  /* It is legal to have orphaned barriers.  */
  if (team == NULL)
    return;
  // 使用线程组内部的 barrier 只有所有的线程都到达这个同步点之后才能够继续往后执行
  // 否则就需要进入内核挂起 
  gomp_team_barrier_wait (&team->barrier);
}
```

上面的代码就是使用当前线程线程组内部的 barrier ，让线程组当中的所有线程都到达同步点之后才继续往后执行，如果你使用过 pthread 中的线程同步工具路障 pthread_barrier_t 的话就很容易理解了。

在继续往后分析程序之前我们首先需要了解两个数据类型：

```c
typedef struct
{
  /* Make sure total/generation is in a mostly read cacheline, while
     awaited in a separate cacheline.  */
  unsigned total __attribute__((aligned (64)));
  unsigned generation;
  unsigned awaited __attribute__((aligned (64)));
} gomp_barrier_t;
typedef unsigned int gomp_barrier_state_t;
```

我们重点分析一下 gomp_barrier_t ，team->barrier 就是这个变量类型，在这个结构体当中一共有三个变量我们重点分析第一个和第三个变量的含义：

- total，这个变量表示一个需要等待多少个线程到达同步点之后才能够继续往后执行。
- awaited，这个变量表示还需要等待多少个线程。
- 初始化的时候 total 和 awaited 这两个变量是相等的。
- generation 这个变量与 OpenMP 当中的 task 有关，这个变量稍微有点复杂，这里不做分析。

现在我们来对函数 gomp_team_barrier_wait 进行分析：

```c
void
gomp_team_barrier_wait (gomp_barrier_t *bar)
{
  gomp_team_barrier_wait_end (bar, gomp_barrier_wait_start (bar));
}

static inline gomp_barrier_state_t
gomp_barrier_wait_start (gomp_barrier_t *bar)
{
  unsigned int ret = __atomic_load_n (&bar->generation, MEMMODEL_ACQUIRE) & ~3;
  /* A memory barrier is needed before exiting from the various forms
     of gomp_barrier_wait, to satisfy OpenMP API version 3.1 section
     2.8.6 flush Construct, which says there is an implicit flush during
     a barrier region.  This is a convenient place to add the barrier,
     so we use MEMMODEL_ACQ_REL here rather than MEMMODEL_ACQUIRE.  */
  ret += __atomic_add_fetch (&bar->awaited, -1, MEMMODEL_ACQ_REL) == 0;
  return ret;
}


// 为了方便阅读下面的代码已经删除了与 task 相关的部分
void
gomp_team_barrier_wait_end (gomp_barrier_t *bar, gomp_barrier_state_t state)
{
  unsigned int generation, gen;

  if (__builtin_expect ((state & 1) != 0, 0))
    {
      /* Next time we'll be awaiting TOTAL threads again.  */
      struct gomp_thread *thr = gomp_thread ();
      struct gomp_team *team = thr->ts.team;

      bar->awaited = bar->total;
      if (__builtin_expect (team->task_count, 0))
	{
	  gomp_barrier_handle_tasks (state);
	  state &= ~1;
	}
      else
	{
	  __atomic_store_n (&bar->generation, state + 3, MEMMODEL_RELEASE);
	  futex_wake ((int *) &bar->generation, INT_MAX);
	  return;
	}
    }
}

```

