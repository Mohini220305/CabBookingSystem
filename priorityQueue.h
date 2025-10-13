#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

typedef struct{
    int vertex;   // place index
    int distance; // current shortest distance
} PQNode;

typedef struct{
    PQNode heap[100];
    int size;
} PriorityQueue;

void initPQ(PriorityQueue *pq);
void push(PriorityQueue *pq, int vertex, int distance);
PQNode pop(PriorityQueue *pq);
int isEmpty(PriorityQueue *pq);

#endif
