# Openmp Runtime 库函数汇总（下）——深入剖析锁🔒原理与实现

## 前言

在本篇文章当中主要给大家介绍一下 OpenMP 当中经常使用到的锁并且仔细分析它其中的内部原理！在 OpenMP 当中主要有两种类型的锁，一个是 omp_lock_t 另外一个是 omp_nest_lock_t，这两个锁的主要区别就是后者是一个可重入锁，所谓可冲入锁就是一旦一个线程已经拿到这个锁了，那么它下一次想要拿这个锁的就是就不会阻塞，但是如果是 omp_lock_t 不管一个线程是否拿到了锁，只要当前锁没有释放，不管哪一个线程都不能够拿到这个锁。在后问当中将有仔细的例子来解释这一点。本篇文章是基于 GNU OpenMP Runtime Library !

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

- omp_unset_lock，这个函数和上面的函数对应，这个函数的主要作用就是用于解锁，在我们调用这个函数之前，必须要使用 omp_set_lock 或者 omp_test_lock 获取锁，它的函数原型如下：

```c
void omp_unset_lock(omp_lock_t *lock);
```

- omp_destroy_lock，这个方法主要是对锁进行回收处理，但是对于这个锁来说是没有用的，我们在后文分析他的具体的实现的时候会发现这是一个空函数。

我们现在使用一个例子来具体的体验一下上面的函数：

```c

#include <stdio.h>
#include <omp.h>

int main()
{
   omp_lock_t lock;
   // 对锁进行初始化操作
   omp_init_lock(&lock);
   int data = 0;
#pragma omp parallel num_threads(16) shared(lock, data) default(none)
   {
      // 进行加锁处理 同一个时刻只能够有一个线程能够获取锁
      omp_set_lock(&lock);
      data++;
      // 解锁处理 线程在出临界区之前需要解锁 好让其他线程能够进入临界区
      omp_unset_lock(&lock);
   }
   omp_destroy_lock(&lock);
   printf("data = %d\n", data);
   return 0;
}
```

在上面的函数我们定义了一个 omp_lock_t 锁，并且在并行域内启动了 16 个线程去执行 data ++ 的操作，因为是多线程环境，因此我们需要将上面的操作进行加锁处理。

## omp_lock_t 源码分析

- omp_init_lock，对于这个函数来说最终在 OpenMP 动态库内部会调用下面的函数：

```c
typedef int gomp_mutex_t;
static inline void
gomp_mutex_init (gomp_mutex_t *mutex)
{
  *mutex = 0;
}
```

从上面的函数我们可以知道这个函数的作用就是将我们定义的 4 个字节的锁赋值为0，这就是锁的初始化，其实很简单。

- omp_set_lock，这个函数最终会调用 OpenMP 内部的一个函数，具体如下所示：

```c
static inline void
gomp_mutex_lock (gomp_mutex_t *mutex)
{
  int oldval = 0;
  if (!__atomic_compare_exchange_n (mutex, &oldval, 1, false,
				    MEMMODEL_ACQUIRE, MEMMODEL_RELAXED))
    gomp_mutex_lock_slow (mutex, oldval);
}
```

在上面的函数当中线程首先会调用 __atomic_compare_exchange_n 将锁的值由 0 变成 1，还记得我们在前面对锁进行初始化的时候将锁的值变成0了吗？

我们首先需要了解一下 __atomic_compare_exchange_n ，这个是  gcc 内嵌的一个函数，在这里我们只关注前面三个参数，后面三个参数与内存模型有关，这并不是我们本篇文章的重点，他的主要功能是查看 mutex 指向的地址的值等不等于 oldval ，如果等于则将这个值变成 1，这一整个操作能够保证原子性，如成功将 mutex 指向的值变成 1 的话，那么这个函数就返回 true 否则返回 false 对应 C 语言的数据就是 1 和 0 。**如果 oldval 的值不等于 mutex 所指向的值，那么这个函数就会将这个值写入 oldval 。**

如果这个操作不成功那么就会调用 gomp_mutex_lock_slow 函数这个函数的主要作用就是如果使用不能够使用原子指令获取锁的话，那么就需要进入内核态，将这个线程挂起。在这个函数的内部还会测试是否能够通过源自操作获取锁，因为可能在我们调用 gomp_mutex_lock_slow 这个函数的时候可能有其他线程释放锁了。如果仍然不能够成功的话，那么就会真正的将这个线程挂起不会浪费 CPU 资源，gomp_mutex_lock_slow 函数具体如下：

```c
void
gomp_mutex_lock_slow (gomp_mutex_t *mutex, int oldval)
{
  /* First loop spins a while.  */
  // 先自旋 如果自旋一段时间还没有获取锁 那就将线程刮挂起
  while (oldval == 1)
    {
      if (do_spin (mutex, 1))
	{
	  /* Spin timeout, nothing changed.  Set waiting flag.  */
	  oldval = __atomic_exchange_n (mutex, -1, MEMMODEL_ACQUIRE);
    // 如果获得🔒 就返回
	  if (oldval == 0)
	    return;
    // 如果没有获得🔒 那么就将线程刮起
	  futex_wait (mutex, -1);
    // 这里是当挂起的线程被唤醒之后的操作 也有可能是 futex_wait 没有成功
	  break;
	}
      else
	{
	  /* Something changed.  If now unlocked, we're good to go.  */
	  oldval = 0;
	  if (__atomic_compare_exchange_n (mutex, &oldval, 1, false,
					   MEMMODEL_ACQUIRE, MEMMODEL_RELAXED))
	    return;
	}
    }

  /* Second loop waits until mutex is unlocked.  We always exit this
     loop with wait flag set, so next unlock will awaken a thread.  */
  while ((oldval = __atomic_exchange_n (mutex, -1, MEMMODEL_ACQUIRE)))
    do_wait (mutex, -1);
}

```

在上面的函数当中有三个依赖函数，他们的源代码如下所示：

```c

static inline void
futex_wait (int *addr, int val)
{
  // 在这里进行系统调用，将线程挂起 
  int err = syscall (SYS_futex, addr, gomp_futex_wait, val, NULL);
  if (__builtin_expect (err < 0 && errno == ENOSYS, 0))
    {
      gomp_futex_wait &= ~FUTEX_PRIVATE_FLAG;
      gomp_futex_wake &= ~FUTEX_PRIVATE_FLAG;
    // 在这里进行系统调用，将线程挂起 
      syscall (SYS_futex, addr, gomp_futex_wait, val, NULL);
    }
}

static inline void do_wait (int *addr, int val)
{
  if (do_spin (addr, val))
    futex_wait (addr, val);
}

static inline int do_spin (int *addr, int val)
{
  unsigned long long i, count = gomp_spin_count_var;

  if (__builtin_expect (__atomic_load_n (&gomp_managed_threads,
                                         MEMMODEL_RELAXED)
                        > gomp_available_cpus, 0))
    count = gomp_throttled_spin_count_var;
  for (i = 0; i < count; i++)
    if (__builtin_expect (__atomic_load_n (addr, MEMMODEL_RELAXED) != val, 0))
      return 0;
    else
      cpu_relax ();
  return 1;
}

static inline void
cpu_relax (void)
{
  __asm volatile ("" : : : "memory");
}
```

如果大家对具体的内部实现非常感兴趣可以仔细研读上面的代码，如果从 0 开始解释上面的代码比较麻烦，这里就不做详细的分析了，简要做一下概括：

- 在锁的设计当中有一个非常重要的原则：一个线程最好不要进入内核态被挂起，如果能够在用户态最好在用户态使用原子指令获取锁，这是因为进入内核态是一个非常耗时的事情相比起原子指令来说。

- 锁（就是我们在前面讨论的一个 4 个字节的 int 类型的值）有以下三个值：
  - -1 表示现在有线程被挂起了。
  - 0  表示现在是一个无锁状态，这个状态就表示锁的竞争比较激烈。
  - 1 表示这个线程正在被一个线程用一个原子指令——比较并交换(CAS)获得了，这个状态表示现在锁的竞争比较轻。
- _atomic_exchange_n (mutex, -1, MEMMODEL_ACQUIRE); ，这个函数也是 gcc 内嵌的一个函数，这个函数的主要作用就是将 mutex 的值变成 -1，然后将 mutex 指向的地址的原来的值返回。
- __atomic_load_n (addr, MEMMODEL_RELAXED)，这个函数的作用主要作用是原子的加载 addr 指向的数据。

- futex_wait 函数的功能是将线程挂起，将线程挂起的系统调用为 futex ，大家可以使用命令 man futex 去查看 futex 的手册。
- do_spin 函数的功能是进行一定次数的原子操作（自旋），如果超过这个次数就表示现在这个锁的竞争比较激烈为了更好的使用 CPU 的计算资源可以将这个线程挂起。如果在自旋（spin）的时候发现锁的值等于 val 那么就返回 0 ，如果在进行 count 次操作之后我们还没有发现锁的值变成 val  那么就返回 1 ，这就表示锁的竞争比较激烈。
- 可能你会疑惑在函数 gomp_mutex_lock_slow 的最后一部分为什么要用 while 循环，这是因为 do_wait 函数不一定会将线程挂起，这个和 futex 系统调用有关，感兴趣的同学可以去看一下 futex 的文档，就了解这么设计的原因了。
- 在上面的源代码当中有两个 OpenMP 内部全局变量，gomp_throttled_spin_count_var 和 gomp_spin_count_var 用于表示自旋的次数，这个也是 OpenMP 自己进行设计的这个值和环境变量 OMP_WAIT_POLICY  也有关系，具体的数值也是设计团队的经验值，在这里就不介绍这一部分的源代码了。

其实上面的加锁过程是非常复杂的，大家可以自己自行去好好分析一下这其中的设计，其实是非常值得学习的，上面的加锁代码贯彻的宗旨就是：能不进内核态就别进内核态。

- omp_unset_lock，这个函数的主要功能就是解锁了，我们再来看一下他的源代码设计。这个函数最终调用的 OpenMP 内部的函数为 gomp_mutex_unlock ，其源代码如下所示：

```c
static inline void
gomp_mutex_unlock (gomp_mutex_t *mutex)
{
  int wait = __atomic_exchange_n (mutex, 0, MEMMODEL_RELEASE);
  if (__builtin_expect (wait < 0, 0))
    gomp_mutex_unlock_slow (mutex);
}
```

在上面的函数当中调用一个函数 gomp_mutex_unlock_slow ，其源代码如下：

```c
void
gomp_mutex_unlock_slow (gomp_mutex_t *mutex)
{
  // 表示唤醒 1 个线程
  futex_wake (mutex, 1);
}

static inline void
futex_wake (int *addr, int count)
{
  int err = syscall (SYS_futex, addr, gomp_futex_wake, count);
  if (__builtin_expect (err < 0 && errno == ENOSYS, 0))
    {
      gomp_futex_wait &= ~FUTEX_PRIVATE_FLAG;
      gomp_futex_wake &= ~FUTEX_PRIVATE_FLAG;
      syscall (SYS_futex, addr, gomp_futex_wake, count);
    }
}
```

在函数 gomp_mutex_unlock 当中首先调用原子操作 __atomic_exchange_n，将锁的值变成 0 也就是无锁状态，这个其实是方便被唤醒的线程能够不被阻塞（关于这一点大家可以好好去分分析 gomp_mutex_lock_slow 最后的 while 循环，就能够理解其中的深意了），然后如果 mutex 原来的值（这个值会被赋值给 wait ）小于 0 ，我们在前面已经谈到过，这个值只能是 -1，这就表示之前有线程进入内核态被挂起了，因此这个线程需要唤醒之前被阻塞的线程，好让他们能够继续执行。唤醒之前线程的函数就是 gomp_mutex_unlock_slow，在这个函数内部会调用 futex_wake 去真正的唤醒一个之前被锁阻塞的线程。

总的说来上面的锁的设计主要有一下的两个方向：

- Fast path : 能够在用户态解决的事儿就别进内核态，只要能够通过原子指令获取锁，那么就使用原子指令，因为进入内核态是一件非常耗时的事情。
- Slow path : 当经历过一定数目的自旋操作之后发现还是不能够获得锁，那么就能够判断此时锁的竞争比较激烈，如果这个时候还不将线程挂起的话，那么这个线程好就会一直消耗 CPU ，因此这个时候我们应该要进入内核态将线程挂起以节省 CPU 的计算资源。
