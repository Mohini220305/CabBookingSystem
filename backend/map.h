#ifndef MAP_H
#define MAP_H
#define MAX 100

#include "driver.h"

// Adjacency List Node
typedef struct Node{
    int dest;
    int distance;
    struct Node *next;
} Node;

// Graph Structure
typedef struct Graph{
    int n;                    // number of locations
    char placeNames[MAX][30]; // names of places
    Node *list[MAX];          // adjacency list
} Graph;


Graph *city;
Graph *createMap(int places);
Graph *initDehradunMap();
Node *createNode(int dest, int distance);
void addRoad(Graph *g, int src, int dest, int distance);
void displayMap(Graph *g);
int findNearestDriver(Graph *g, Driver drivers[], int numDrivers, int pickupLoc, int excludeDriverId);
int getLocationIndex(Graph *g, char placeName[]);
int calculateDistance(Graph *city, int pickupIdx, int dropIdx);

#endif