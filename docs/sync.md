# OpenMP 线程同步 construct 实现原理以及源码分析

## 前言

在本篇文章当中主要给大家介绍在 OpenMP 当中使用的一些同步的 construct 的实现原理，如 master, single, critical 等等！并且会结合对应的汇编程序进行仔细的分析。（本篇文章的汇编程序分析基于 x86_86 平台）

## Flush Construct

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

上面两种方式的编译姐若干的主要区别就是一个使用 lock 指令，一个使用 mfence 指令，实际上 lock 的效率比 mfence 效率更高因此在很多场景下，现在都是使用 lock 指令进行实现。

在我的机器上下面的代码分别使用 gcc-11 和 gcc-4 编译之后执行的结果差异很大，gcc-11 大约使用了 11 秒，而 gcc-4 编译出来的结果执行了 20 秒，这其中主要的区别就是 lock 指令和 mfence 指令的差异。

```c

#include <stdio.h>
#include <omp.h>

int main()
{
  double start = omp_get_wtime();
  for(long i = 0; i < 1000000000; ++i)
  {
    __sync_synchronize();
  }
  printf("time = %lf\n", omp_get_wtime() - start);
  return 0;
}
```

