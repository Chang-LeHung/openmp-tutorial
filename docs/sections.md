# OpenMP Section Construct 实现原理以及源码分析

## 前言

在本篇文章当中主要分析 sections construct 的实现原理，主要分析这个 construct 的编译之后的代码是怎么样的，以及他调用的库函数。

## 编译器角度分析

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