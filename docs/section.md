# OpenMP Sections Construct 实现原理以及源码分析

## 前言

在本篇文章当中主要给大家介绍 OpenMP 当中主要给大家介绍 OpenMP 当中 sections construct 的实现原理以及他调用的动态库函数分析。如果已经了解过了前面的关于 for 的调度方式的分析，本篇文章就非常简单了。

## 编译器角度分析

在这一小节当中我们将从编译器角度去分析编译器会怎么处理 sections construct ，我们以下面的 sections construct 为例子，看看编译器是如何处理 sections construct 的。

```c
#pragma omp sections
{
  #pragma omp section
  stmt1;
  #pragma omp section
  stmt2;
  #pragma omp section
  stmt3;
}
```

上面的代码会被编译器转换成下面的形式，其中 GOMP_sections_start 和 GOMP_sections_next 是并发安全的，他们都会返回一个数据表示第几个 omp section 代码块，其中 GOMP_sections_start 的参数是表示有几个 omp section 代码块，并且返回给线程一个整数表示线程需要执行第几个 section 代码块，这两个函数的意义不同的是在 GOMP_sections_start 当中会进行一些数据的初始化操作。当两个函数返回 0 的时候表示所有的 section 都被执行完了，从而退出 for 循环。

```c
for (i = GOMP_sections_start (3); i != 0; i = GOMP_sections_next ())
  switch (i)
    {
    case 1:
      stmt1;
      break;
    case 2:
      stmt2;
      break;
    case 3:
      stmt3;
      break;
    }
GOMP_barrier ();
```

## 动态库函数分析



```c
unsigned
GOMP_sections_start (unsigned count)
{
  struct gomp_thread *thr = gomp_thread ();
  long s, e, ret;

  if (gomp_work_share_start (false))
    {
      gomp_sections_init (thr->ts.work_share, count);
      gomp_work_share_init_done ();
    }
  if (gomp_iter_dynamic_next (&s, &e))
    ret = s;
  else
    ret = 0;
  return ret;
}


unsigned
GOMP_sections_next (void)
{
  long s, e, ret;
  if (gomp_iter_dynamic_next (&s, &e))
    ret = s;
  else
    ret = 0;
  return ret;
}

bool
gomp_iter_dynamic_next (long *pstart, long *pend)
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_work_share *ws = thr->ts.work_share;
  long start, end, nend, chunk, incr;

  end = ws->end;
  incr = ws->incr;
  chunk = ws->chunk_size;

  if (__builtin_expect (ws->mode, 1))
    {
      long tmp = __sync_fetch_and_add (&ws->next, chunk);
      if (incr > 0)
	{
	  if (tmp >= end)
	    return false;
	  nend = tmp + chunk;
	  if (nend > end)
	    nend = end;
	  *pstart = tmp;
	  *pend = nend;
	  return true;
	}
      else
	{
	  if (tmp <= end)
	    return false;
	  nend = tmp + chunk;
	  if (nend < end)
	    nend = end;
	  *pstart = tmp;
	  *pend = nend;
	  return true;
	}
    }

  start = ws->next;
  while (1)
    {
      long left = end - start;
      long tmp;

      if (start == end)
	return false;

      if (incr < 0)
	{
	  if (chunk < left)
	    chunk = left;
	}
      else
	{
	  if (chunk > left)
	    chunk = left;
	}
      nend = start + chunk;

      tmp = __sync_val_compare_and_swap (&ws->next, start, nend);
      if (__builtin_expect (tmp == start, 1))
	break;

      start = tmp;
    }

  *pstart = start;
  *pend = nend;
  return true;
}
```

