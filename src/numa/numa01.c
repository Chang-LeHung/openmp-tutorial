
#include <stdio.h>
#include <omp.h>
#include <utmpx.h>
#include <unistd.h>
#include <sched.h>


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
  int cpuid, nodeid;
  tacc_rdtscp(&nodeid, &cpuid);
  printf("cpuid = %d nodeid = %d\n", cpuid, nodeid);
  int cpu = sched_getcpu();
  printf("CPU: %d\n", cpu);
  return 0;
}