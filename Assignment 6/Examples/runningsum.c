#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int sum = 0, count = 0;

void *runner(void *arg)
{
    for (int i = 0; i < 100; i++)
    {
        count++;
        sum += count;
    }
}

int main()
{
    pthread_t tid;
    
    pthread_create(&tid, NULL, runner, NULL);

    pthread_join(tid, NULL);

    printf("Sum: %d\n", sum);

    return 0;
}