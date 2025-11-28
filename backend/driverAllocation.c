#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "ride.h"
#include "map.h"
#include "driver.h"
#include "priorityQueue.h"

#define CUSTOMER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt"
#define RIDES_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt"
#define DRIVER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt"

// ======================== LOAD DRIVERS ========================
int loadDrivers(Driver drivers[])
{
    FILE *fp = fopen(DRIVER_FILE, "rb");
    if (!fp)
    {
        printf("<p><b>Warning:</b> drivers.txt not found! No drivers loaded.</p>");
        return 0;
    }

    int count = 0;
    while (count < 50 && fread(&drivers[count], sizeof(Driver), 1, fp) == 1)
        count++;

    fclose(fp);
    return count;
}

// ======================== UPDATE SINGLE DRIVER ========================
void updateDriverAvailability(int driverId, int available)
{
    FILE *fp = fopen(DRIVER_FILE, "rb+");
    if (!fp)
    {
        printf("<p style='color:red;'>Error opening drivers file!</p>");
        return;
    }

    Driver d;
    while (fread(&d, sizeof(Driver), 1, fp) == 1)
    {
        if (d.id == driverId)
        {
            d.available = available;
            fseek(fp, -sizeof(Driver), SEEK_CUR);
            fwrite(&d, sizeof(Driver), 1, fp);
            fflush(fp); // Important for CGI immediate write
            break;
        }
    }
    fclose(fp);
}

// ======================== FIND NEAREST DRIVER (exclude option) ========================
int findNearestDriver(Graph *g, Driver drivers[], int numDrivers, int pickupLoc, int excludeDriverId)
{
    int dist[MAX], visited[MAX];
    for (int i = 0; i < g->n; i++)
    {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }

    PriorityQueue pq;
    initPQ(&pq);
    push(&pq, pickupLoc, 0);
    dist[pickupLoc] = 0;

    while (!isEmpty(&pq))
    {
        PQNode curr = pop(&pq);
        int u = curr.vertex;
        if (visited[u])
            continue;
        visited[u] = 1;

        Node *temp = g->list[u];
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

    int nearestDriver = -1;
    int minDistance = INT_MAX;
    for (int i = 0; i < numDrivers; i++)
    {
        // Exclude requested driver and only consider available drivers
        if (drivers[i].id == excludeDriverId)
            continue;

        if (drivers[i].available == 1)
        {
            if (drivers[i].location >= 0 && drivers[i].location < g->n)
            {
                // dist[...] could be INT_MAX if unreachable
                if (dist[drivers[i].location] < minDistance)
                {
                    minDistance = dist[drivers[i].location];
                    nearestDriver = drivers[i].id;
                }
            }
            else
            {
                // debug: driver has invalid location index
                // printf("<p>Debug: driver %d has invalid location %d (g->n=%d)</p>", drivers[i].id, drivers[i].location, g->n);
            }
        }
    }

    if (nearestDriver == -1)
        printf("<p>No available drivers nearby (excluding id=%d)!</p>", excludeDriverId);
    else
        printf("<p>Nearest driver found: %d (Distance: %d km)</p>", nearestDriver, minDistance);

    return nearestDriver;
}

// ======================== ALLOCATE DRIVER ========================
// Added excludeDriverId parameter so allocateDriver can exclude a specific driver (like a rejected one)
int allocateDriver(Graph *city, Driver drivers[], int numDrivers, int pickupLoc, char *pickup, char *drop, int excludeDriverId)
{
    int driverId = -1;
    int sameRouteCount = 0;

    // Check existing rides for pooling (same pickup/drop)
    FILE *rfp = fopen(RIDES_FILE, "rb");
    if (rfp)
    {
        Ride existing;
        while (fread(&existing, sizeof(Ride), 1, rfp) == 1)
        {
            // skip excluded driver
            if (existing.driverId == excludeDriverId)
                continue;

            if (strcmp(existing.pickup, pickup) == 0 &&
                strcmp(existing.drop, drop) == 0 &&
                existing.driverId != -1 &&
                strcmp(existing.status, "Pending") == 0)
            {
                if (driverId == -1)
                    driverId = existing.driverId;
                if (existing.driverId == driverId)
                    sameRouteCount++;
            }
        }
        fclose(rfp);
    }

    // Pooling limit: max 2 rides per same route
    if (sameRouteCount >= 2)
    {
        // If pooling limit reached, ignore pooling and find nearest driver (excluding anyone in excludeDriverId)
        driverId = -1;
    }

    // If no pooling driver chosen, find nearest available driver (excluding excludeDriverId)
    if (driverId == -1)
        driverId = findNearestDriver(city, drivers, numDrivers, pickupLoc, excludeDriverId);

    // Update availability in file and memory
    if (driverId != -1)
    {
        updateDriverAvailability(driverId, 0);

        for (int i = 0; i < numDrivers; i++)
        {
            if (drivers[i].id == driverId)
            {
                drivers[i].available = 0;
                break;
            }
        }
    }

    return driverId;
}

// ======================== REASSIGN RIDE (reload drivers + exclude rejected) ========================
void reassignRide(Ride *ride, Graph *city, int rejectedDriverId)
{
    Driver drivers[50];
    int numDrivers = loadDrivers(drivers);

    // Mark rejected driver available again in file
    updateDriverAvailability(rejectedDriverId, 1);

    // --- KEY FIX: reload drivers so in-memory array matches file ---
    numDrivers = loadDrivers(drivers);

    // Also update the in-memory rejected driver's availability (redundant but safe)
    for (int i = 0; i < numDrivers; i++)
    {
        if (drivers[i].id == rejectedDriverId)
        {
            drivers[i].available = 1;
            break;
        }
    }

    int pickupIndex = getLocationIndex(city, ride->pickup);

    // Pass the rejectedDriverId so it will not be reselected
    int newDriverId = allocateDriver(city, drivers, numDrivers, pickupIndex, ride->pickup, ride->drop, rejectedDriverId);

    printf("<div style='margin-top:20px;'>");

    if (newDriverId != -1)
    {
        ride->driverId = newDriverId;
        strcpy(ride->status, "Pending");

        printf("<p style='color:green; font-weight:bold;'>✅ Ride reassigned successfully!</p>");
        printf("<p>New Driver ID: <b>%d</b></p>", newDriverId);
    }
    else
    {
        ride->driverId = -1;
        strcpy(ride->status, "Waiting");
        enqueueRide(ride);
        printf("<p style='color:orange; font-weight:bold;'>⚠️ No nearby drivers available. Ride moved to Waiting queue.</p>");
    }

    printf("</div>");

    // Update ride details in rides file
    FILE *fp = fopen(RIDES_FILE, "rb+");
    if (!fp)
    {
        printf("<p style='color:red;'>Error updating rides file!</p>");
        return;
    }

    Ride temp;
    while (fread(&temp, sizeof(Ride), 1, fp) == 1)
    {
        if (temp.rideID == ride->rideID)
        {
            fseek(fp, -sizeof(Ride), SEEK_CUR);
            fwrite(ride, sizeof(Ride), 1, fp);
            fflush(fp); // Ensure write happens immediately
            break;
        }
    }
    fclose(fp);
}
