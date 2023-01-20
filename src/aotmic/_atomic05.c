

#include <stdio.h>
#include <stdbool.h>
#include <stdatomic.h>

void atomic_multiply(int* data)
{
  int oldval = *data;
  int write = oldval * 2;
  while (__atomic_compare_exchange_n (data, &oldval, write, false,
                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED));
}

int main()
{
  int data = 2;
  atomic_multiply(&data);
  printf("data = %d\n", data);
  return 0;
}