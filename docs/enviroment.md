# Openmp Runtime 库函数汇总

## 并行域相关

- omp_in_parallel，如果当前线程正在并行域内不，则此函数返回true，否则返回false。

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

- omp_get_active_level，这个函数主要是返回当前线程所在的并行域的嵌套的并行块的级别，从 1 开始，从外往内没加一层这个值就加一。