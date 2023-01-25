
#include <stdio.h>
#include <omp.h>

int main()
{
  int data = 100;
#pragma omp parallel num_threads(4) default(none) shared(data)
  {
#pragma omp flush(data)
  }
  return 0;
}