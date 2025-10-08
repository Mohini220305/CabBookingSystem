#include "priorityQueue.h"

void initPQ(PriorityQueue *pq)
{
    pq->size = 0;
}

void push(PriorityQueue *pq, int vertex, int distance)
{
    int i = pq->size++;
    while (i > 0 && distance < pq->heap[(i - 1) / 2].distance)
    {
        pq->heap[i] = pq->heap[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    pq->heap[i].vertex = vertex;
    pq->heap[i].distance = distance;
}

PQNode pop(PriorityQueue *pq)
{
    PQNode minNode = pq->heap[0];
    PQNode lastNode = pq->heap[--pq->size];
    int i = 0, child;

    while (2 * i + 1 < pq->size)
    {
        child = 2 * i + 1;
        if (child + 1 < pq->size && pq->heap[child + 1].distance < pq->heap[child].distance)
            child++;
        if (lastNode.distance <= pq->heap[child].distance)
            break;
        pq->heap[i] = pq->heap[child];
        i = child;
    }
    pq->heap[i] = lastNode;
    return minNode;
}

int isEmpty(PriorityQueue *pq)
{
    return pq->size == 0;
}

