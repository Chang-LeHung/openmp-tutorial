# OpenMP For Construct guided 调度方式实现原理和源码分析

## 前言

在本篇文章当中主要给大家介绍在 OpenMP 当中 guided 调度方式的实现原理。这个调度方式其实和 dynamic 调度方式非常相似的，从编译器角度来说基本上是一样的，在本篇文章当中就不介绍一些相关的必备知识了，如果不了解可以先看这篇文章 [OpenMP For Construct dynamic 调度方式实现原理和源码分析](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487775&idx=1&sn=112f5fb600584bdd4a7acfeddb58cd6e&chksm=cf0c8d16f87b040098e650d350dce82b1fa75549c05ba25d92c8a4da3da661cfcef5a5c9542c&token=1250927684&lang=zh_CN#rd) 。

## guided 调度方式分析

我们使用下面的代码来分析一下 guided 调度的情况下整个程序的执行流程是怎么样的：

```c
#pragma omp parallel for num_threads(t) schedule(dynamic, size)
for (i = lb; i <= ub; i++)
  body;
```

编译器会将上面的程序编译成下面的形式：

```c
void subfunction (void *data)
{
  long _s0, _e0;
  while (GOMP_loop_guided_next (&_s0, &_e0))
  {
    long _e1 = _e0, i;
    for (i = _s0; i < _e1; i++)
      body;
  }
  // GOMP_loop_end_nowait 这个函数的主要作用就是释放数据的内存空间 在后文当中不进行分析
  GOMP_loop_end_nowait ();
}

GOMP_parallel_loop_guided_start (subfunction, NULL, t, lb, ub+1, 1, size);
subfunction (NULL);
// 这个函数在前面的很多文章已经分析过 本文也不在进行分析
GOMP_parallel_end ();
```

根据上面的代码可以知道，上面的代码当中最主要的两个函数就是 GOMP_parallel_loop_guided_start 和 GOMP_loop_guided_next，现在我们来分析一下他们的源代码：

- GOMP_parallel_loop_guided_start

```c
void
GOMP_parallel_loop_guided_start (void (*fn) (void *), void *data,
				 unsigned num_threads, long start, long end,
				 long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_GUIDED, chunk_size);
}

static void
gomp_parallel_loop_start (void (*fn) (void *), void *data,
			  unsigned num_threads, long start, long end,
			  long incr, enum gomp_schedule_type sched,
			  long chunk_size)
{
  struct gomp_team *team;

  num_threads = gomp_resolve_num_threads (num_threads, 0);
  team = gomp_new_team (num_threads);
  gomp_loop_init (&team->work_shares[0], start, end, incr, sched, chunk_size);
  gomp_team_start (fn, data, num_threads, team);
}

```

在上面的程序当中 GOMP_parallel_loop_guided_start，有 7 个参数，我们接下来仔细解释一下这七个参数的含义：

- fn，函数指针也就是并行域被编译之后的函数。
- data，指向共享或者私有的数据，在并行域当中可能会使用外部的一些变量。
- num_threads，并行域当中指定启动线程的个数。
- start，for 循环迭代的初始值，比如 for(int i = 0; ;) 这个 start 就是 0 。
- end，for 循环迭代的最终值，比如 for(int i = 0; i < 100; i++) 这个 end 就是 100 。
- incr，这个值一般都是 1 或者 -1，如果是 for 循环是从小到达迭代这个值就是 1，反之就是 -1，实际上这个值指的是 for 循环 i 大的增量。
- chunk_size，这个就是给一个线程划分块的时候一个块的大小，比如 schedule(dynamic, 1)，这个 chunk_size 就等于 1 。

事实上上面的代码和 GOMP_parallel_loop_dynamic_start 基本上一摸一样，函数参数也一直，唯一的区别就是调度方式的不同。

