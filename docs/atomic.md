# OpenMP 原子指令设计与实现

## 前言

在本篇文章当中主要与大家分享一下 openmp 当中的原子指令 atomic，分析 `#pragma omp atomic` 在背后究竟做了什么，编译器是如何处理这条指令的。

## 为什么需要原子指令

加入现在有两个线程分别执行在 CPU0 和 CPU1，如果这两个线程都要对同一个共享变量进行更新操作，就会产生竞争条件。如果没有保护机制来避免这种竞争，可能会导致结果错误或者程序崩溃。原子指令就是解决这个问题的一种解决方案，它能够保证操作的原子性，即操作不会被打断或者更改。这样就能保证在多线程环境下更新共享变量的正确性。

比如在下面的图当中，两个线程分别在 CPU0 和 CPU1 执行 data++ 语句，如果目前主存当中的 data = 1 ，然后按照图中的顺序去执行，那么主存当中的 data 的最终值等于 2 ，但是这并不是我们想要的结果，因为有两次加法操作我们希望最终在内存当中的 data 的值等于 3 ，那么有什么方法能够保证一个线程在执行 data++ 操作的时候下面的三步操作是原子的嘛（不可以分割）：

- Load data : 从主存当中将 data 加载到 cpu 的缓存。
- data++ : 执行 data + 1 操作。
- Store data : 将 data 的值写回主存。

事实上硬件就给我们提供了这种机制，比如 x86 的 lock 指令，在这里我们先不去讨论这一点，我们将在后文当中对此进行仔细的分析。

![](../images/07.png)

## OpenMP 原子指令

在 openmp 当中 `#pragma omp atomic` 的表达式格式如下所示：

```c
#pragma omp atomic
表达式;
```

其中表达式可以是一下几种形式：

```c
x binop = 表达式;
x++;
x--;
++x;
--x;
```

二元运算符 binop 为++， --， +， -， *， /， &， ^， | ， >>， <<或 || ，x 是基本数据类型 int，short，long，float 等数据类型。

 我们现在来使用一个例子熟悉一下上面锁谈到的语法：

```c


#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 1;
#pragma omp parallel num_threads(4) shared(data) default(none)
  {
#pragma omp atomic
    data += data * 2;
  }
  printf("data = %d\n", data);
  return 0;
}
```

上面的程序最终的输出结果如下：

```shell
data = 81
```

上面的 data += data * 2 ，相当于每次操作将 data 的值扩大三倍，因此最终的结果就是 81 。

## 原子操作和锁的区别

OpenMP 中的 atomic 指令允许执行无锁操作，而不会影响其他线程的并行执行。这是通过在硬件层面上实现原子性完成的。锁则是通过软件来实现的，它阻塞了其他线程对共享资源的访问。

在选择使用 atomic 或锁时，应该考虑操作的复杂性和频率。对于简单的操作和高频率的操作，atomic 更加高效，因为它不会影响其他线程的并行执行。但是，对于复杂的操作或者需要多个操作来完成一个任务，锁可能更加合适。

原子操作只能够进行一些简单的操作，如果操作复杂的是没有原子指令进行操作的，这一点我们在后文当中详细谈到，如果你想要原子性的是一个代码块的只能够使用锁，而使用不了原子指令。

## 深入剖析原子指令——从汇编角度

我们现在来仔细分析一下下面的代码的汇编指令，看看编译器在背后为我们做了些什么：

```c

#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 0;
#pragma omp parallel num_threads(4) shared(data) default(none)
  {
#pragma omp atomic
    data += 1;
  }
  printf("data = %d\n", data);
  return 0;
}
```

首先我们需要了解一点编译器会将并行域的代码编译成一个函数，我们现在看看上面的 parallel 并行域的对应的函数的的汇编程序：

```asm
0000000000401193 <main._omp_fn.0>:
  401193:       55                      push   %rbp
  401194:       48 89 e5                mov    %rsp,%rbp
  401197:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  40119b:       48 8b 45 f8             mov    -0x8(%rbp),%rax
  40119f:       48 8b 00                mov    (%rax),%rax
  4011a2:       f0 83 00 01             lock addl $0x1,(%rax) # 这就是编译出来的原子指令——对应x86平台
  4011a6:       5d                      pop    %rbp
  4011a7:       c3                      retq   
  4011a8:       0f 1f 84 00 00 00 00    nopl   0x0(%rax,%rax,1)
  4011af:       00 
```

