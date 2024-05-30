#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

pthread_t tid[2];
int counter = 0;

void* doSomeThing(void *arg)
{
    unsigned long i = 0;
    counter += 1;

    printf("\n Job %d started\n", counter);
    for(i=0; i<(0xFFFFFFF);i++);
    printf("\n Job %d finished\n", counter);

    return NULL;
}

int main(void)
{
    int i = 0;
    int err;

    int numCPU = sysconf(_SC_NPROCESSORS_ONLN);

    while(i < numCPU)
    {
        err = pthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        i++;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    return 0;
}
