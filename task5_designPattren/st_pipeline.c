#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "activeobject.h"

// part a:
int isPrime(unsigned int num)
{
    if (num == 2)
        return 1;
    if (num < 2 || num % 2 == 0)
        return 0;
    for (unsigned int i = 3; i <= sqrt(num); i += 2)
    {
        if (num % i == 0)
        {
            return 0;
        }
    }
    return 1;
}

// part d:

void partD_a(void *arg)
{
    void **arr = (void **)arg;
    int size = *(int *)arr[0];
    int seed = *(int *)arr[1];
    pao ao_1 = (pao)arr[2];
    srand(seed);
    int i = 0;
    while (i < size)
    {
        int num = rand() % 900000 + 100000;
        enqueue(ao_1->next->queue, (void *)&num);
        sleep(1);
        i++;
    }
    enqueue(ao_1->queue, NULL);
}

// partD_b
void partD_b(void* arg)
{
    int* num = (int*)arg;
    *num += 11;
    printf("%d\n%s\n", *num, isPrime(*num) ? "true" : "false");
}

// partD_c
void partD_c(void* arg)
{
    int* num = (int*)arg;
    *num -= 13;
    printf("%d\n%s\n", *num, isPrime(*num) ? "true" : "false");
}

// partD_d
void partD_d(void* arg)
{
    int* num = (int*)arg;
    printf("%d\n%s\n", *num, isPrime(*num) ? "true" : "false");
    *num += 2;
    printf("%d\n", *num);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        perror("more elements need\n");
        return 1;
    }
    int len = atoi(argv[1]);
    int seed = argc > 2 ? atoi(argv[2]) : time(NULL);
    void **arr = (void **)malloc(sizeof(void *) * 3);

    if (arr == NULL)
    {
        fprintf(stderr, "The memory allocation failed\n");
        return 1;
    }
    arr[0] = (void *)&len;
    arr[1] = (void *)&seed;
    pao ao_4 = createActiveObject(partD_d, NULL);
    pao ao_3 = createActiveObject(partD_c, ao_4);
    pao ao_2 = createActiveObject(partD_b, ao_3);
    pao ao_1 = createActiveObject(partD_a, ao_2);
    ao_1->flag = 1;
    arr[2] = ao_1;
    enqueue(ao_1->queue, (void *)arr);
    sleep(1);
    pthread_join(ao_1->thread, NULL);
    pthread_join(ao_2->thread, NULL);
    pthread_join(ao_3->thread, NULL);
    pthread_join(ao_4->thread, NULL);
    stop(ao_1);
    stop(ao_2);
    stop(ao_3);
    stop(ao_4);
    free(arr);
    return 0;
}