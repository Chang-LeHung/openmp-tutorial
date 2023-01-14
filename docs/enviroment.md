# Openmp Runtime 库函数汇总（上）

## 并行域相关

- omp_in_parallel，如果当前线程正在并行域内部，则此函数返回true，否则返回false。

```c

#include <stdio.h>
#include <omp.h>

int main()
{
   printf("0> Not In parallel region value = %d\n",
          omp_in_parallel());

   // 在这里函数返回的 false 在 C 语言当中返回值等于 int 类型的 0
   if (omp_in_parallel())
   {
      printf("1> In parallel region value = %d\n",
             omp_in_parallel());
   }
   #pragma omp parallel num_threads(2) default(none)
   {
      // 这里函数的返回值是 1 在 C 语言当中对应 int 类型的 1
      if (omp_in_parallel())
      {
         printf("2> In parallel region value = %d\n",
                omp_in_parallel());
      }
   }
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
0> Not In parallel region value = 0
2> In parallel region value = 1
2> In parallel region value = 1
```

需要注意的是在上面的函数 omp_in_parallel 使用时需要注意，如果你的并行域只有一个线程的时候 omp_in_parallel 返回的是 false。比如下面的例子：

```c

#include <stdio.h>
#include <omp.h>

int main()
{
   printf("0> Not In parallel region value = %d\n",
          omp_in_parallel());

   // 在这里函数返回的 false 在 C 语言当中返回值等于 int 类型的 0
   if (omp_in_parallel())
   {
      printf("1> In parallel region value = %d\n",
             omp_in_parallel());
   }
  // 只有一个线程，因此并不会激活并行域
   #pragma omp parallel num_threads(1) default(none)
   {
      // 这里函数的返回值是 1 在 C 语言当中对应 int 类型的 1
      if (omp_in_parallel())
      {
         printf("2> In parallel region value = %d\n",
                omp_in_parallel());
      }
   }
   return 0;
}
```

在上面的程序当中，因为并行域当中只有一个线程，因此不会激活，所以即使在 parallel 代码块内不，这个 omp_in_parallel 也不会被触发，上面的程序只会输出第一个 printf 的内容：

```shell
0> Not In parallel region value = 0
```

- omp_get_thread_num，这个函数的主要作用是用于返回当前并行域的线程组当中的唯一 ID，在一个串行代码当中，这个函数的返回结果之中等于 0 ，在并行域当中这个函数的返回值的区间等于 [0, num_threads - 1]，如果是一个线程组的 master 线程调用这个函数，这个函数的返回值始终是 0，对应的测试代码如下所示：

```c

#include <stdio.h>
#include <omp.h>

int main()
{
   printf("Non parallel region: omp_get_thread_num() = %d\n", omp_get_thread_num());

#pragma omp parallel num_threads(5) default(none)
   {
      printf("In parallel region: omp_get_thread_num() = %d\n", omp_get_thread_num());
#pragma omp master
      {
         printf("In parallel region master: omp_get_thread_num() = %d\n", omp_get_thread_num());
      }
   }

   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
Non parallel region: omp_get_thread_num() = 0
In parallel region: omp_get_thread_num() = 0
In parallel region master: omp_get_thread_num() = 0
In parallel region: omp_get_thread_num() = 4
In parallel region: omp_get_thread_num() = 1
In parallel region: omp_get_thread_num() = 2
In parallel region: omp_get_thread_num() = 3
```

在上面的代码当中我们可以看到，在非并行域当中（第一个 printf） 函数的输出结果是 0，在并行域当中程序的输出结果范围是 [0. num_threads]，如果一个线程是 master 线程的话，那么它对应的线程组当中的返回值就是0。

- omp_get_team_size，这个函数的主要作用就是返回一个线程组当中的线程个数。这个函数接受一个参数，他的函数原型为 `int omp_get_team_size(int level);`，这个参数的 level 的含义就是并行域的嵌套层级，这个参数的范围是 [0, omp_get_level]，如果传入的参数不在这个范围，那么这个函数的返回值为 -1，如果你在一个并行域传入的 level 的参数是  omp_get_level ，那么这个函数的返回值和 omp_get_num_threads 的返回值相等。

```c

#include <stdio.h>
#include <omp.h>

int main()
{
   omp_set_nested(1);
   int level = omp_get_level();
   int size = omp_get_team_size(level);
   printf("Non parallel region : level = %d size = %d\n", level, size);

#pragma omp parallel num_threads(5) default(none)
   {
      int level = omp_get_level();
      int size = omp_get_team_size(level);
      printf("level = %d size = %d\n", level, size);

#pragma omp parallel num_threads(2) default(none)
      {
         int level = omp_get_level();
         int size = omp_get_team_size(level);
         printf("level = %d size = %d\n", level, size);

         printf("Up one level team size = %d\n", omp_get_team_size(level - 1));
      }
   }
   return 0;
}
```

上面的程序的输出结果如下所示：

```shell
Non parallel region : level = 0 size = 1
level = 1 size = 5
level = 1 size = 5
level = 1 size = 5
level = 1 size = 5
level = 1 size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
level = 2 size = 2
Up one level team size = 5
```



- omp_get_active_level，这个函数主要是返回当前线程所在的并行域的嵌套的并行块的级别，从 1 开始，从外往内没加一层这个值就加一。

```c


#include <stdio.h>
#include <omp.h>


int main()
{
   #pragma omp parallel num_threads(2) default(none)
   {
      printf("Outer tid = %d leve = %d num_threads = %d\n",
             omp_get_thread_num(), omp_get_active_level(),
             omp_get_num_threads());
      #pragma omp parallel num_threads(2) default(none)
      {
         printf("Outer tid = %d leve = %d num_threads = %d\n",
                omp_get_thread_num(), omp_get_active_level(),
                omp_get_num_threads());
      }
   }
   return 0;
}
```

```shell
Outer tid = 0 leve = 1 num_threads = 2
Outer tid = 0 leve = 1 num_threads = 1
Outer tid = 1 leve = 1 num_threads = 2
Outer tid = 0 leve = 1 num_threads = 1
```

当我们启动嵌套并行域之后我们再看程序的输出结果：

```c


#include <stdio.h>
#include <omp.h>


int main()
{
   omp_set_nested(1);
   #pragma omp parallel num_threads(2) default(none)
   {
      printf("Outer tid = %d leve = %d num_threads = %d\n",
             omp_get_thread_num(), omp_get_active_level(),
             omp_get_num_threads());
      #pragma omp parallel num_threads(2) default(none)
      {
         printf("Outer tid = %d leve = %d num_threads = %d\n",
                omp_get_thread_num(), omp_get_active_level(),
                omp_get_num_threads());
      }
   }
   return 0;
}
```

```shell
Outer tid = 0 leve = 1 num_threads = 2
Outer tid = 1 leve = 1 num_threads = 2
Outer tid = 0 leve = 2 num_threads = 2
Outer tid = 1 leve = 2 num_threads = 2
Outer tid = 0 leve = 2 num_threads = 2
Outer tid = 1 leve = 2 num_threads = 2
```

