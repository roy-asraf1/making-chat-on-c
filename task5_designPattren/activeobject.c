#include <stdint.h>
#include "activeobject.h"

//part c
void *run(void *arg)
{
    pao this = (pao)arg;
    void *temp;
    for (temp = dequeue(this->queue); temp != NULL; temp = dequeue(this->queue))
    {
        this->func(temp);
        if (this->flag != 1 && this->next != NULL)
        {
            enqueue(getQueue(this->next), temp);
        }
    }
    if (this->next != NULL)
        enqueue(getQueue(this->next), temp);
    return NULL;
}

pao createActiveObject(void (*func)(void *), pao next)
{
    pao temp = (pao)malloc(sizeof(ao));
    if (temp == NULL)
        return NULL;
    temp->queue = createQueue();
    temp->next = next;
    temp->func = func;
    pthread_create(&temp->thread, NULL, run, temp);
    return temp;
}

pqueue getQueue(pao obj)
{
    if (obj == NULL)
        return NULL;
    return obj->queue;
}

void stop(pao obj)
{
    if (obj == NULL)
        return;
    pthread_join(obj->thread, NULL);
    deleteQueue(obj->queue);
    free(obj);
}
