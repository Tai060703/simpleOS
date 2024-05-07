#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
    if (q == NULL)
        return 1;
    if (q->rear == q->front) return 1;
    return 0;
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
    /* TODO: put a new process to queue [q] */
    if (q->rear == q->size)
        return;
    q->proc[q->rear] = proc;
    q->rear++;
}

struct pcb_t *dequeue(struct queue_t *q)
{
    /* TODO: return a pcb whose prioprity is the highest
     * in the queue [q] and remember to remove it from q
     * */
    struct pcb_t *result = q->proc[q->front];
    if (q->front == q->rear)
        return result = NULL;
    for (int i = 0; i < q->rear - 1; i++)
    {
        q->proc[i] = q->proc[i + 1];
    }
    q->rear--;
    return result;
}