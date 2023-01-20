

#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 0;
#pragma omp parallel num_threads(4) shared(data) default(none)
  {
#pragma omp atomic
    data += 1;
  }
  printf("data = %d\n", data);
  return 0;
}