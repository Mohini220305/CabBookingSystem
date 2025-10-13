#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "ride.h"
#include "map.h"
#include "driver.h"
#include "priorityQueue.h"

int findNearestDriver(Graph *g, Driver drivers[], int numDrivers, int pickupLoc){
    int dist[MAX], visited[MAX];
    for (int i = 0; i < g->n; i++){
        dist[i] = INT_MAX;
        visited[i] = 0;
    }

    PriorityQueue pq;
    initPQ(&pq);
    push(&pq, pickupLoc, 0);
    dist[pickupLoc] = 0;

    while (!isEmpty(&pq)){
        PQNode curr = pop(&pq);
        int u = curr.vertex;
        if (visited[u])
            continue;
        visited[u] = 1;

        Node *temp = g->list[u];
        while (temp){
            int v = temp->dest;
            int newDist = dist[u] + temp->distance;
            if (newDist < dist[v]){
                dist[v] = newDist;
                push(&pq, v, newDist);
            }
            temp = temp->next;
        }
    }

    int nearestDriver = -1;
    int minDistance = INT_MAX;
    for (int i = 0; i < numDrivers; i++){
        if (drivers[i].available == 1 && dist[drivers[i].location] < minDistance){
            minDistance = dist[drivers[i].location];
            nearestDriver = drivers[i].id;
        }
    }

    if (nearestDriver == -1)
        printf("No available drivers nearby!\n");
    else
        printf("Nearest driver found: %d (Distance: %d km)\n", nearestDriver, minDistance);

    return nearestDriver;
}

int allocateDriver(Graph *city, Driver drivers[], int numDrivers, int pickupLoc, char *pickup, char *drop){
    numDrivers = loadDrivers(drivers);
    int driverId = -1;
    int sameRouteCount = 0;

    FILE *rfp = fopen("rides.txt", "rb");
    if (rfp){
        Ride existing;
        while (fread(&existing, sizeof(Ride), 1, rfp)){
            if (strcmp(existing.pickup, pickup) == 0 && strcmp(existing.drop, drop) == 0 && existing.driverId != -1){
                if (driverId == -1)
                    driverId = existing.driverId;
                if (existing.driverId == driverId)
                    sameRouteCount++;
            }
        }
        fclose(rfp);
    }

    if (sameRouteCount >= 2){
        driverId = -1;
    }

    if (driverId == -1){
        driverId = findNearestDriver(city, drivers, numDrivers, pickupLoc);
    }

    if (driverId != -1){
        FILE *dfp = fopen("drivers.txt", "rb+");
        if (dfp){
            Driver d;
            while (fread(&d, sizeof(Driver), 1, dfp)){
                if (d.id == driverId){
                    d.available = 0;
                    fseek(dfp, -sizeof(Driver), SEEK_CUR);
                    fwrite(&d, sizeof(Driver), 1, dfp);
                    break;
                }
            }
            fclose(dfp);
        }

        for (int i = 0; i < numDrivers; i++){
            if (drivers[i].id == driverId){
                drivers[i].available = 0;
                break;
            }
        }
    }
    return driverId;
}

void reassignRide(Ride *ride, Graph *city, int rejectedDriverId){
    Driver drivers[50];
    int numDrivers = loadDrivers(drivers);

    for (int i = 0; i < numDrivers; i++){
        if (drivers[i].id == rejectedDriverId){
            drivers[i].available = 0;
            break;
        }
    }

    int pickupIndex = getLocationIndex(city, ride->pickup);
    int driverId = allocateDriver(city, drivers, numDrivers, pickupIndex, ride->pickup, ride->drop);

    if (driverId == rejectedDriverId){
        driverId = -1;
    }

    if (driverId != -1){
        ride->driverId = driverId;
        strcpy(ride->status, "Pending");
        printf("Ride reassigned to Driver ID: %d\n", driverId);
    }
    else{
        ride->driverId = -1;
        strcpy(ride->status, "Waiting");
        printf("No available drivers nearby. Ride moved to Waiting queue.\n");
    }

    FILE *fp = fopen("rides.txt", "rb+");
    if (!fp){
        printf("Error updating rides file!\n");
        return;
    }

    Ride temp;
    while (fread(&temp, sizeof(Ride), 1, fp) == 1){
        if (temp.rideID == ride->rideID){
            fseek(fp, -sizeof(Ride), SEEK_CUR);
            fwrite(ride, sizeof(Ride), 1, fp);
            break;
        }
    }
    fclose(fp);
}
