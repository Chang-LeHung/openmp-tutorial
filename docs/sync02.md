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
- generation 这个变量与 OpenMP 当中的 task 有关，这个变量稍微有点复杂，由于我们的分析不涉及到 OpenMP 当中的任务，因此这类对这个变量不做分析，这个变量的初始值等于 0。

结构体 gomp_barrier_t 初始化函数如下所示：

```c
static inline void gomp_barrier_init (gomp_barrier_t *bar, unsigned count)
{
  bar->total = count;
  bar->awaited = count;
  bar->generation = 0;
}
```

现在我们来对函数 gomp_team_barrier_wait 进行分析，关于代码的详细都在代码的对应位置：

```c
void
gomp_team_barrier_wait (gomp_barrier_t *bar)
{
  gomp_team_barrier_wait_end (bar, gomp_barrier_wait_start (bar));
}

static inline gomp_barrier_state_t
gomp_barrier_wait_start (gomp_barrier_t *bar)
{
  // 因为我们不分析 OpenMP 当中的 task ,因此在这里可能认为 generation 始终等于 0 
  // 那么 ret 也等于 0
  unsigned int ret = __atomic_load_n (&bar->generation, MEMMODEL_ACQUIRE) & ~3;
  /* A memory barrier is needed before exiting from the various forms
     of gomp_barrier_wait, to satisfy OpenMP API version 3.1 section
     2.8.6 flush Construct, which says there is an implicit flush during
     a barrier region.  This is a convenient place to add the barrier,
     so we use MEMMODEL_ACQ_REL here rather than MEMMODEL_ACQUIRE.  */
  // 这里将 awaited 还需要等待的线程数 -1 并且判断 awaited 是否等于 0
  // 如果等于 0 则返回 1 反之则返回 0 如果不考虑 task 只有最后一个到达同步点的线程
  // 才会返回 1
  ret += __atomic_add_fetch (&bar->awaited, -1, MEMMODEL_ACQ_REL) == 0;
  return ret;
}


// 为了方便阅读下面的代码已经删除了与 task 相关的部分
void
gomp_team_barrier_wait_end (gomp_barrier_t *bar, gomp_barrier_state_t state)
{
  unsigned int generation, gen;
	// 如果 state 等于 1 将会进入下面的 if 语句
  if (__builtin_expect ((state & 1) != 0, 0))
    {
    	// 如果是最后一个线程到达这里，那么将会重新将 awaited 变成 total
      /* Next time we'll be awaiting TOTAL threads again.  */
      struct gomp_thread *thr = gomp_thread ();
      struct gomp_team *team = thr->ts.team;

      bar->awaited = bar->total;
    // 如果还有需要执行的任务 那么将进入 if 语句
      if (__builtin_expect (team->task_count, 0))
	{
	  gomp_barrier_handle_tasks (state);
	  state &= ~1;
	}
      else
	{
    // 如果没有需要执行的任务 那么则需要将之前被挂起的线程全部唤醒
	  __atomic_store_n (&bar->generation, state + 3, MEMMODEL_RELEASE);
	  futex_wake ((int *) &bar->generation, INT_MAX);
	  return;
	}
    }
  // 如果 if 条件不满足，也就是说到达 barrier 的线程不是最后一个线程
  // 那么将会执行到这里进行挂起
  
  // 这里省略了代码 如果程序执行到这里将会被继续挂起 直到上面的 futex_wake 被执行
}

```

### 技巧分析

- 在上面的结构体 gomp_barrier_t 当中有语句 `unsigned total __attribute__((aligned (64)));` 后面的 __attribute__((aligned (64))) 表示这个字段需要使用 64 字节对齐，那么这个字段也占 64 字节，一般来说一个缓存行有 64 个字节的数据，也就是说这三个字段的数据不会存储在同一个缓存行，这样的话多个线程在操作这三个数据的时候不会产生假共享 (false sharing) 的问题，这可以很提高程序的效率。

- 我们在前面讨论 critical construct 的时候谈到啦 critical 有匿名和命令两种方式：

```c
#pragma omp critical
#pragma omp critical(name)
```

那么按道理来说 barrier 也应该有两种方式啊，那么为什么会没有呢？根据前面的程序分析，我们可以知道，最重要的一行代码是 `gomp_team_barrier_wait (&team->barrier);` 因为每一个线程都属于一个线程组，每个线程组内部都有一个 barrier ，因此当进行同步的时候只需要使用线程组内部的 barrier 即可，因此不需要使用命名的 barrier。

## Single Construct

在本小节当中我们主要分析 single construct ，他的一半形式如下所示：

```c
#pragma omp single
{
  body;
}
```

类似于上面的结构的代码会被编译器编译成如下形式：

```c
if (GOMP_single_start ())
  body;
GOMP_barrier ();
```

关于 GOMP_barrier 函数我们在前面的内容当中已经进行了详细的分析，他的功能就是使用一个线程组内部的 barrier 变量，当所有的线程都到达这个位置之后才放行所有线程，让他们继续执行，如果线程组的线程没有全部到达同步点，则到达同步点的线程会被挂起。

我们使用一个实际的例子进行分析，看一下最终被编译成的程序是什么样子：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp single
    {
      printf("Hello World\n");
    }

    printf("tid = %d\n", omp_get_thread_num());
  }
  return 0;
}
```

上面的 parallel 代码块被编译之后的反汇编程序如下所示：

```asm
00000000004011aa <main._omp_fn.0>:
  4011aa:       55                      push   %rbp
  4011ab:       48 89 e5                mov    %rsp,%rbp
  4011ae:       48 83 ec 10             sub    $0x10,%rsp
  4011b2:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  4011b6:       e8 c5 fe ff ff          callq  401080 <GOMP_single_start@plt>
  4011bb:       3c 01                   cmp    $0x1,%al
  4011bd:       74 1d                   je     4011dc <main._omp_fn.0+0x32>
  4011bf:       e8 7c fe ff ff          callq  401040 <GOMP_barrier@plt>
  4011c4:       e8 87 fe ff ff          callq  401050 <omp_get_thread_num@plt>
  4011c9:       89 c6                   mov    %eax,%esi
  4011cb:       bf 10 20 40 00          mov    $0x402010,%edi
  4011d0:       b8 00 00 00 00          mov    $0x0,%eax
  4011d5:       e8 86 fe ff ff          callq  401060 <printf@plt>
  4011da:       eb 0c                   jmp    4011e8 <main._omp_fn.0+0x3e>
  4011dc:       bf 1a 20 40 00          mov    $0x40201a,%edi
  4011e1:       e8 4a fe ff ff          callq  401030 <puts@plt>
  4011e6:       eb d7                   jmp    4011bf <main._omp_fn.0+0x15>
  4011e8:       c9                      leaveq 
  4011e9:       c3                      retq   
  4011ea:       66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)
```

从上面的汇编程序我们可以看到，被编译的程序确实调用了函数 GOMP_single_start，如果这个函数的返回值不等于 true 的时候就会执行函数 GOMP_barrier 。这和我们上面的分析是一样的。

现在最主要的函数就是 GOMP_single_start ，他的源代码如下所示：

```c
bool
GOMP_single_start (void)
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_team *team = thr->ts.team;
  unsigned long single_count;

  if (__builtin_expect (team == NULL, 0))
    return true;

  single_count = thr->ts.single_count++;
  return __sync_bool_compare_and_swap (&team->single_count, single_count,
				       single_count + 1L);
}
```

