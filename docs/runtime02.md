# Openmp Runtime 库函数汇总（下）——锁🔒

## 前言

在本篇文章当中主要给大家介绍一下 OpenMP 当中经常使用到的锁并且仔细分析它其中的内部原理！在 OpenMP 当中主要有两种类型的锁，一个是 omp_lock_t 另外一个是 omp_nest_lock_t，这两个锁的主要区别就是后者是一个可重入锁，所谓可冲入锁就是一旦一个线程已经拿到这个锁了，那么它下一次想要拿这个锁的就是就不会阻塞，但是如果是 omp_lock_t 不管一个线程是否拿到了锁，只要当前锁没有释放，不管哪一个线程都不能够拿到这个锁。在后问当中将有仔细的例子来解释这一点。

## 深入分析 omp_lock_t

这是 OpenMP 头文件给我们提供的一个结构体，我们来看一下它的定义：

```c
typedef struct
{
  unsigned char _x[4] 
    __attribute__((__aligned__(4)));
} omp_lock_t;
```

事实上这个结构体并没有什么特别的就是占 4 个字节，我们甚至可以认为他就是一个 4 字节的 int 的类型的变量，只不过使用方式有所差异。与这个结构体相关的主要有以下几个函数：

- omp_init_lock，这个函数的主要功能是初始化 omp_lock_t 对象的，当我们初始化之后，这个锁就处于一个没有上锁的状态，他的函数原型如下所示：

```c
void omp_init_lock(omp_lock_t *lock);
```

- omp_set_lock，在调用这个函数之前一定要先调用函数 omp_init_lock 将 omp_lock_t 进行初始化，直到这个锁被释放之前这个线程会被一直阻塞。如果这个锁被当前线程已经获取过了，那么将会造成一个死锁，这就是上面提到了锁不能够重入的问题，而我们在后面将要分析的锁 omp_nest_lock_t 是能够进行重入的，即使当前线程已经获取到了这个锁，也不会造成死锁而是会重新获得锁。这个函数的函数原型如下所示：

```c
void omp_set_lock(omp_lock_t *lock);
```

- omp_test_lock，这个函数的主要作用也是用于获取锁，但是这个函数可能会失败，如果失败就会返回 false 成功就会返回 true，与函数 omp_set_lock 不同的是，这个函数并不会导致线程被阻塞，如果获取锁成功他就会立即返回 true，如果失败就会立即返回 false 。它的函数原型如下所示：

```c
int omp_test_lock(omp_lock_t *lock); 
```

- omp_unset_lock，这个函数和上面的函数对应，这个函数的主要作用就是用于解锁，它的函数原型如下：

```c
void omp_unset_lock(omp_lock_t *lock);
```

我们现在使用一个例子来具体的体验一下上面的函数：

```c

#include <stdio.h>
#include <omp.h>

int main()
{
   omp_lock_t lock;
   omp_init_lock(&lock);
   int data = 0;
#pragma omp parallel num_threads(16) shared(lock, data) default(none)
   {
      omp_set_lock(&lock);
      data++;
      omp_unset_lock(&lock);
   }
   omp_destroy_lock(&lock);
   printf("data = %d\n", data);
   return 0;
}
```

在上面的函数我们定义了一个 omp_lock_t 锁，并且在并行域内启动了 16 个线程去执行 data ++ 的操作，因为是多线程环境，因此我们需要将上面的操作进行加锁处理。

