# OpenMP Parallel Construct 实现原理与源码分析

## 前言

在本篇文章当中我们将主要分析 OpenMP 当中的 parallel construct 具体时如何实现的，以及这这个 construct 调用了哪些运行时库函数！

## Parallel 分析——编译器角度

在本小节当中我们将从编译器的角度去分析该如何处理 parallel construct 。首先从词法分析和语法分析的角度来说这对编译器并不难，只需要加上一些处理规则，关键是编译器将一个 parallel construct 具体编译成了什么？

下面是一个非常简单的

```c
#pragma omp parallel
{
  body;
}
```

```c
void subfunction (void *data)
{
  use data;
  body;
}

setup data;
GOMP_parallel_start (subfunction, &data, num_threads);
subfunction (&data);
GOMP_parallel_end ();
```

## 深入剖析 Parallel 动态库函数参数传递

```c

#include <stdio.h>
#include "omp.h"

int main()
{
  int data = 100;
  int two  = -100;
  printf("start\n");
#pragma omp parallel num_threads(4) default(none) shared(data, two)
  {
    printf("tid = %d data = %d two = %d\n", omp_get_thread_num(), data, two);
  }

  printf("finished\n");
  return 0;
}
```



上面的代码当中两个变量 `data` 和 `two` 在内存当中的布局结构大致如下所示（假设 data 的初始位置时 0x0）：

![](../images/12.png)

![](../images/13.png)

| 寄存器 | 含义       |
| ------ | ---------- |
| rdi    | 第一个参数 |
| rsi    | 第二个参数 |
| rdx    | 第三个参数 |
| rcx    | 第四个参数 |
| r8     | 第五个参数 |
| r9     | 第六个参数 |



```asm
00000000004006cd <main>:
  4006cd:       55                      push   %rbp
  4006ce:       48 89 e5                mov    %rsp,%rbp
  4006d1:       48 83 ec 10             sub    $0x10,%rsp
  4006d5:       c7 45 fc 64 00 00 00    movl   $0x64,-0x4(%rbp)
  4006dc:       bf f0 07 40 00          mov    $0x4007f0,%edi
  4006e1:       e8 9a fe ff ff          callq  400580 <puts@plt>
  4006e6:       8b 45 fc                mov    -0x4(%rbp),%eax
  4006e9:       89 45 f0                mov    %eax,-0x10(%rbp)
  4006ec:       48 8d 45 f0             lea    -0x10(%rbp),%rax
  4006f0:       ba 04 00 00 00          mov    $0x4,%edx
  4006f5:       48 89 c6                mov    %rax,%rsi
  4006f8:       bf 2a 07 40 00          mov    $0x40072a,%edi
  4006fd:       e8 6e fe ff ff          callq  400570 <GOMP_parallel_start@plt>
  400702:       48 8d 45 f0             lea    -0x10(%rbp),%rax
  400706:       48 89 c7                mov    %rax,%rdi
  400709:       e8 1c 00 00 00          callq  40072a <main._omp_fn.0>
  40070e:       e8 7d fe ff ff          callq  400590 <GOMP_parallel_end@plt>
  400713:       8b 45 f0                mov    -0x10(%rbp),%eax
  400716:       89 45 fc                mov    %eax,-0x4(%rbp)
  400719:       bf f6 07 40 00          mov    $0x4007f6,%edi
  40071e:       e8 5d fe ff ff          callq  400580 <puts@plt>
  400723:       b8 00 00 00 00          mov    $0x0,%eax
  400728:       c9                      leaveq 
  400729:       c3                      retq 
  
  
000000000040072a <main._omp_fn.0>:
  40072a:       55                      push   %rbp
  40072b:       48 89 e5                mov    %rsp,%rbp
  40072e:       48 83 ec 10             sub    $0x10,%rsp
  400732:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  400736:       e8 65 fe ff ff          callq  4005a0 <omp_get_thread_num@plt>
  40073b:       48 8b 55 f8             mov    -0x8(%rbp),%rdx
  40073f:       8b 12                   mov    (%rdx),%edx
  400741:       89 c6                   mov    %eax,%esi
  400743:       bf ff 07 40 00          mov    $0x4007ff,%edi
  400748:       b8 00 00 00 00          mov    $0x0,%eax
  40074d:       e8 5e fe ff ff          callq  4005b0 <printf@plt>
  400752:       c9                      leaveq 
  400753:       c3                      retq   
  400754:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  40075b:       00 00 00 
  40075e:       66 90                   xchg   %ax,%ax

```
