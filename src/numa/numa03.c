

#include <stdio.h>
#include <omp.h>
#include <unistd.h>
#include <stdlib.h>

#define N 16

unsigned long tacc_rdtscp(int *chip, int *core)
{
  unsigned long a,d,c;
  __asm__ volatile("rdtscp" : "=a" (a), "=d" (d), "=c" (c));
  *chip = (c & 0xFFF000) >>12;
  *core = c & 0xFFF;
  return ((unsigned long)a) | (((unsigned long)d) << 32);;
}


int main()
{
  char* messages[N];
#pragma omp parallel num_threads(N) default(none) shared(messages)
  {
    char * msg = malloc(1024);
    const int tid = omp_get_thread_num();
    int cpuid, nodeid;
    tacc_rdtscp(&nodeid, &cpuid);
    char s[1024];
    gethostname(s, 1024);
    sprintf(msg, "hostname = %s cpuid = %d nodeid = %d\n", s, cpuid, nodeid);
    messages[tid] = msg;
  }
  for(int i = 0; i < N; ++i)
  {
    printf("%s", messages[i]);
  }
  return 0;
}