#include <pthread.h>
#include <stdio.h>  // For printf()
#include <stdlib.h> // For exit()
#include <string.h> // For strlen()
#include <unistd.h> // For sleep() if needed

int count;

static void
    *
    code_for_thread1(void *arg)
{
    printf("count in thread 1 is initially %d\n", count);
    count = 20;
    printf("count in thread 1 is finally   %d\n", count);
    return NULL;
}

int main(void)
{
    pthread_t thread1;
    void *thread1_result;

    count = 10;
    printf("count in main thread is initially %d\n", count);

    pthread_create(&thread1, NULL, code_for_thread1, NULL);
    pthread_join(thread1, &thread1_result);

    printf("count in main thread is finally   %d\n", count);
    exit(0);
}