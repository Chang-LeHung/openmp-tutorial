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
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_team *team = thr->ts.team;

  /* It is legal to have orphaned barriers.  */
  if (team == NULL)
    return;

  gomp_team_barrier_wait (&team->barrier);
}
```

