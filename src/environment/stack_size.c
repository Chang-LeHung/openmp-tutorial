
#include <stdio.h>
#include <omp.h>
#include <pthread.h>

int main()
{

#pragma omp parallel num_threads(2) default(none)
   {

   }

   return 0;
}