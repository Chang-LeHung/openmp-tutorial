# OpenMP 线程同步 construct 实现原理以及源码分析

## 前言

在本篇文章当中主要给大家介绍在 OpenMP 当中使用的一些同步的 construct 的实现原理，如 master, single, critical 等等！并且会结合对应的汇编程序进行仔细的分析。（本篇文章的汇编程序分析基于 x86_86 平台）

## Flush Construct

首先先了解一下 flush construct 的语法：

```c
#pragma omp flush(变量列表)
```

这个构造比较简单，其实就是增加一个内存屏障，保证多线程环境下面的数据的可见性，简单来说一个线程对某个数据进行修改之后，修改之后的结果对其他线程来说是可见的。

```c

#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 100;
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
#pragma omp flush(data)
  }
  return 0;
}
```

上面是一个非常简单的 OpenMP 的程序，根据前面的文章 [OpenMp Parallel Construct 实现原理与源码分析](https://github.com/Chang-LeHung/openmp-tutorial/blob/master/docs/parallel.md) 我们可以知道会讲并行域编译成一个函数，我们现在来看一下这个编译后的汇编程序是怎么样的！

gcc-4 编译之后的结果

```asm
00000000004005f6 <main._omp_fn.0>:
  4005f6:       55                      push   %rbp
  4005f7:       48 89 e5                mov    %rsp,%rbp
  4005fa:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  4005fe:       0f ae f0                mfence 
  400601:       5d                      pop    %rbp
  400602:       c3                      retq   
  400603:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  40060a:       00 00 00 
  40060d:       0f 1f 00                nopl   (%rax)
```

从上面的结果我们可以看到最终的一条指令是 mfence 这是一条 full 的内存屏障，用于保障数据的可见性，主要是 cache line 中数据的可见性。

gcc-11 编译之后的结果

```asm
0000000000401165 <main._omp_fn.0>:
  401165:       55                      push   %rbp
  401166:       48 89 e5                mov    %rsp,%rbp
  401169:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  40116d:       f0 48 83 0c 24 00       lock orq $0x0,(%rsp)
  401173:       5d                      pop    %rbp
  401174:       c3                      retq   
  401175:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  40117c:       00 00 00 
  40117f:       90                      nop
```

从编译之后的结果来看，这个汇编程序主要是使用 lock 指令实现可见性，我们知道 lock 指令是用来保证原子性的，但是事实上这同样也保证来可见性，试想一下如果不保证可见性是不能够保证原子性的！因为如果这个线程看到的数据都不是最新修改的数据的话，那么即使操作是原子的那么也达不到我们想要的效果。

上面两种方式的编译姐若干的主要区别就是一个使用 lock 指令，一个使用 mfence 指令，实际上 lock 的效率比 mfence 效率更高因此在很多场景下，现在都是使用 lock 指令进行实现。

在我的机器上下面的代码分别使用 gcc-11 和 gcc-4 编译之后执行的结果差异很大，gcc-11 大约使用了 11 秒，而 gcc-4 编译出来的结果执行了 20 秒，这其中主要的区别就是 lock 指令和 mfence 指令的差异。

```c

#include <stdio.h>
#include <omp.h>

int main()
{
  double start = omp_get_wtime();
  for(long i = 0; i < 1000000000L; ++i)
  {
    __sync_synchronize();
  }
  printf("time = %lf\n", omp_get_wtime() - start);
  return 0;
}
```

## Master Construct

master construct 的使用方法如下所示：

```c
#pragma omp master
```

事实上编译器会将上面的编译指导语句编译成与下面的代码等价的汇编程序：

```c
if (omp_get_thread_num () == 0)
  block // master 的代码块
```

我们现在来分析一个实际的例子，看看程序编译之后的结果是什么：

```c
#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel num_threads(4) default(none)
  {
#pragma omp master
    {
      printf("I am master and my tid = %d\n", omp_get_thread_num());
    }
  }
  return 0;
}
```

上面的程序编译之后的结果如下所示（汇编程序的大致分析如下）：

```asm
000000000040117a <main._omp_fn.0>:
  40117a:       55                      push   %rbp
  40117b:       48 89 e5                mov    %rsp,%rbp
  40117e:       48 83 ec 10             sub    $0x10,%rsp
  401182:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
  401186:       e8 a5 fe ff ff          callq  401030 <omp_get_thread_num@plt> # 得到线程的 id 并保存到 eax 寄存器当中
  40118b:       85 c0                   test   %eax,%eax # 看看寄存器 eax 是不是等于 0
  40118d:       75 16                   jne     4011a5  <main._omp_fn.0+0x2b> # 如果不等于 0 则跳转到 4011a5 的位置 也就是直接退出程序了 如果是那么就继续执行后面的 printf 语句
  40118f:       e8 9c fe ff ff          callq  401030 <omp_get_thread_num@plt>
  401194:       89 c6                   mov    %eax,%esi
  401196:       bf 10 20 40 00          mov    $0x402010,%edi
  40119b:       b8 00 00 00 00          mov    $0x0,%eax
  4011a0:       e8 9b fe ff ff          callq  401040 <printf@plt>
  4011a5:       90                      nop
  4011a6:       c9                      leaveq 
  4011a7:       c3                      retq   
  4011a8:       0f 1f 84 00 00 00 00    nopl   0x0(%rax,%rax,1)
  4011af:       00 
```

这里我们只需要了解一下 test 指令就能够理解上面的汇编程序了，"test %eax, %eax" 是 x86 汇编语言中的一条指令，它的含义是对寄存器 EAX 和 EAX 进行逻辑与运算，并将结果存储在状态寄存器中，但是不改变 EAX 的值。这条指令会影响标志位（如 ZF、SF、OF），可用于判断 EAX 是否等于零。

从上面的汇编程序分析我们也可以知道，master constcut 就是一条 if 语句，但是后面我们将要谈到的 single 不一样他还需要进行同步。

## 