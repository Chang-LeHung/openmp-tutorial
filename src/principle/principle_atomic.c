
#include <stdio.h>


int main()
{
  int data = 0;
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
#pragma omp atomic
    data++;
  }
  printf("data = %d\n", data);
  return 0;
}