#ifndef QUEUE_H
#define QUEUE_H
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

typedef struct Node
{
    struct Node *next;
    void *task;
} Node, *pnode;

typedef struct Queue
{
    pnode head;
    pnode tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Queue, *pqueue;

int isEmpty(pqueue queue);
pqueue createQueue();
void deleteQueue(pqueue queue);
void enqueue(pqueue queue, void *task);
void * dequeue(pqueue queue);

#endif