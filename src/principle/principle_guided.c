

#include <stdio.h>
#include <omp.h>

int main()
{
#pragma omp parallel for num_threads(4) default(none) schedule(guided, 10)
  for(int i = 0; i < 100; i += 5)
  {
    printf("i = %d tid = %d\n", i, omp_get_thread_num());
  }

  printf("sizeof(unsigned long long) = %ld\n", sizeof (unsigned long long));
  return 0;
}