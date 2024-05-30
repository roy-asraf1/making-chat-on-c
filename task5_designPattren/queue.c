// part b
#include "queue.h"
#include <time.h>

pqueue createQueue()
{
    pqueue queue = malloc(sizeof(Queue));
    if (queue == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for the queue\n");
        exit(1);
    }

    queue->head = NULL;
    queue->tail = NULL;
    
    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
    {
        fprintf(stderr, "Failed to initialize mutex\n");
        exit(1);
    }

    if (pthread_cond_init(&queue->cond, NULL) != 0)
    {
        fprintf(stderr, "Failed to initialize condition variable\n");
        exit(1);
    }

    return queue;
}
void deleteQueue(pqueue queue)
{
    if (queue == NULL)
    {
        fprintf(stderr, "Invalid queue\n");
        return;
    }

    pthread_mutex_lock(&queue->mutex);

    pnode current;
    for (current = queue->head; current != NULL; current = current->next)
    {
        pnode next = current->next;
        free(current);
        current = next;
    }
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    free(queue);
}

void enqueue(pqueue queue, void *task)
{
    pnode node = malloc(sizeof(Node));
    if (node == NULL)
    {
        exit(1);
    }
    node->task = task;
    node->next = NULL;

    pthread_mutex_lock(&queue->mutex);

    if (queue->head == NULL)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        queue->tail->next = node;
        queue->tail = node;
    }

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

void *dequeue(pqueue queue)
{
    if (queue == NULL)
    {
        fprintf(stderr, "Invalid queue\n");
        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);

    for (; queue->head == NULL;)
    {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    pnode node = queue->head;
    void *memory = node->task;
    queue->head = node->next;
    if (queue->head == NULL)
    {
        queue->tail = NULL;
    }

    free(node);
    pthread_mutex_unlock(&queue->mutex);
    return memory;
}

int isEmpty(pqueue queue)
{
    if (queue == NULL)
    {
        return 1; // Treat invalid queue as empty
    }

    pthread_mutex_lock(&queue->mutex);
    int result = (queue->head == NULL);
    pthread_mutex_unlock(&queue->mutex);
    return result;
}

