

#include <stdio.h>
#include <omp.h>
#include <unistd.h>
#include <stdlib.h>

#define N 1024

unsigned long tacc_rdtscp(int *chip, int *core)
{
  unsigned long a,d,c;
  __asm__ volatile("rdtscp" : "=a" (a), "=d" (d), "=c" (c));
  *chip = (c & 0xFFF000) >> 12;
  *core = c & 0xFFF;
  return ((unsigned long)a) | (((unsigned long)d) << 32);;
}


int main()
{
  char* messages[N];
  int nThreads;
#pragma omp parallel default(none) shared(messages, nThreads)
  {
    nThreads = omp_get_num_threads();
    char * msg = malloc(1024);
    const int tid = omp_get_thread_num();
    int cpuid, nodeid;
    tacc_rdtscp(&nodeid, &cpuid);
    char s[1024];
    gethostname(s, 1024);
    sprintf(msg, "Thread[%d/%d]hostname = %s cpuid = %d nodeid = %d\n", tid, nThreads , s, cpuid, nodeid);
    messages[tid] = msg;
    int data = 0;
    while (data > 800000000)
      data++;
  }
  for(int i = 0; i < nThreads; ++i)
  {
    printf("%s", messages[i]);
  }
  return 0;
}