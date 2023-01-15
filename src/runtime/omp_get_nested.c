
#include <stdio.h>
#include <omp.h>

int main()
{
   printf("omp_get_nested() = %d\n", omp_get_nested());
   omp_set_nested(1);
   printf("omp_get_nested() = %d\n", omp_get_nested());
   return 0;
}