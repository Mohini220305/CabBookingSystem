#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "map.h"
#include "driver.h"
#include "ride.h"
#include "priorityQueue.h"

// Create Map
Graph *createMap(int places){
    Graph *g = (Graph *)malloc(sizeof(Graph));
    g->n = places;
    for (int i = 0; i < places; i++){
        g->list[i] = NULL;
    }
    return g;
}

// Create a new node
Node *createNode(int dest, int distance){
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->dest = dest;
    newNode->distance = distance;
    newNode->next = NULL;
    return newNode;
}

// Add road
void addRoad(Graph *g, int src, int dest, int distance)
{
    Node *newNode = createNode(dest, distance);
    newNode->next = g->list[src];
    g->list[src] = newNode;

    newNode = createNode(src, distance);
    newNode->next = g->list[dest];
    g->list[dest] = newNode;
}

// Display Map
void displayMap(Graph *g){
    printf("\n\tCity Map :\n\n");
    for (int i = 0; i < g->n; i++){
        printf("%s -> ", g->placeNames[i]);
        Node *temp = g->list[i];
        while (temp){
            printf("%s(%d km) -> ", g->placeNames[temp->dest], temp->distance);
            temp = temp->next;
        }
        printf("NULL\n");
    }
}

int getLocationIndex(Graph *g, char placeName[]){
    for (int i = 0; i < g->n; i++){
        if (strcmp(g->placeNames[i], placeName) == 0)
            return i;
    }
    return -1;
}

Graph *initDehradunMap(){
    Graph *city = createMap(6);

    strcpy(city->placeNames[0], "Clock Tower");
    strcpy(city->placeNames[1], "ISBT");
    strcpy(city->placeNames[2], "Rajpur Road");
    strcpy(city->placeNames[3], "Ballupur");
    strcpy(city->placeNames[4], "Prem Nagar");
    strcpy(city->placeNames[5], "Clement Town");

    addRoad(city, 0, 1, 5);
    addRoad(city, 0, 2, 3);
    addRoad(city, 1, 3, 6);
    addRoad(city, 2, 3, 2);
    addRoad(city, 3, 4, 4);
    addRoad(city, 4, 5, 7);
    addRoad(city, 1, 5, 10);

    return city;
}

int calculateDistance(Graph *city, int pickupIdx, int dropIdx){
    int dist[MAX], visited[MAX];
    for (int i = 0; i < city->n; i++){
        dist[i] = INT_MAX;
        visited[i] = 0;
    }

    PriorityQueue pq;
    initPQ(&pq);
    push(&pq, pickupIdx, 0);
    dist[pickupIdx] = 0;

    while (!isEmpty(&pq)){
        PQNode curr = pop(&pq);
        int u = curr.vertex;
        if (visited[u])
            continue;
        visited[u] = 1;

        Node *temp = city->list[u];
        while (temp)
        {
            int v = temp->dest;
            int newDist = dist[u] + temp->distance;
            if (newDist < dist[v])
            {
                dist[v] = newDist;
                push(&pq, v, newDist);
            }
            temp = temp->next;
        }
    }

    if (dist[dropIdx] == INT_MAX){
        printf("No route found between %s and %s!\n", city->placeNames[pickupIdx], city->placeNames[dropIdx]);
        return 0;
    }
    return dist[dropIdx];
}
