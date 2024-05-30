#ifndef AO_H
#define AO_H
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "queue.h"
#include <math.h>
#include <stdio.h>
#include <pthread.h>



typedef struct AO
{
    pqueue queue;
    pthread_t thread;
    void (*func)( void*);
    struct AO* next;  
    int flag;
}ao, *pao;


void* run(void* arg);
pao createActiveObject( void (*func)(void*), pao next);
pqueue getQueue(pao obj);
void stop(pao obj);

#endif