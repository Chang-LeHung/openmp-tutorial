# OpenMP Section Construct 实现原理以及源码分析

## 前言

在本篇文章当中主要给大家介绍 OpenMP 当中主要给大家介绍 OpenMP 当中 sections construct 的实现原理以及他调用的动态库函数分析。如果已经了解过了前面的关于 for 的调度方式的分析，本篇文章就非常简单了。

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

