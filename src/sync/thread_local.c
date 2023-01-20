
#include <pthread.h>
#include <stdio.h>

__thread int thread_local_variable = 0;

void* thread_function(void* arg) {
   thread_local_variable++;
   printf("Thread-local variable value in thread: %d\n", thread_local_variable);
   return NULL;
}

int main() {
   pthread_t thread1, thread2;
   pthread_create(&thread1, NULL, thread_function, NULL);
   pthread_create(&thread2, NULL, thread_function, NULL);
   pthread_join(thread1, NULL);
   pthread_join(thread2, NULL);
   printf("Thread-local variable value in main: %d\n", thread_local_variable);
   return 0;
}
