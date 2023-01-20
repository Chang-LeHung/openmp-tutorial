

#include <stdio.h>
#include <stdbool.h>
#include <stdatomic.h>

// 这个函数对应上面的汇编程序
void atomic_multiply(int* data)
{
  int oldval = *data;
  int write = oldval * 2;
  // __atomic_compare_exchange_n 这个函数的作用就是
  // 将 data 指向的值和 old 的值进行比较，如果相等就将 write 的值写入 data
  // 指向的内存地址 如果操作成功返回 true 否则返回 false
  while (!__atomic_compare_exchange_n (data, &oldval, write, false,
                                       __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
  {
    oldval = *data;
    write = oldval * 2;
  }
}

int main()
{
  int data = 2;
  atomic_multiply(&data);
  printf("data = %d\n", data);
  return 0;
}