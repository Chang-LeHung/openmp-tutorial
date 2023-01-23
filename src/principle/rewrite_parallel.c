

#include "pthread.h"
#include "stdio.h"
#include "stdint.h"

typedef struct data_in_main_function{
    int data;
    int two;
}data_in_main_function;

pthread_t threads[4];

void subfunction(void* data)
{
  int two = ((data_in_main_function*)data)->two;
  int data_ = ((data_in_main_function*)data)->data;
  printf("tid = %ld data = %d two = %d\n", pthread_self(), data_, two);
}

int main()
{
  // 在主函数申请 8 个字节的栈空间
  data_in_main_function data;
  data.data = 100;
  data.two = -100;
  for(int i = 0; i < 4; ++i)
  {
    pthread_create(&threads[i], NULL, subfunction, &data);
  }
  for(int i = 0; i < 4; ++i)
  {
    pthread_join(threads[i], NULL);
  }
  return 0;
}