#include "billing.h"
#include "ride.h"
#include "priorityQueue.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"

RideStack rs;
BillStack billStack = {.top = -1};

void initBillStack()
{
    billStack.top = -1;
}

int isBillStackEmpty()
{
    return billStack.top == -1;
}

int isBillStackFull()
{
    return billStack.top == MAX_BILLS - 1;
}

void pushBill(Ride *ride)
{
    if (isBillStackFull())
    {
        printf("Bill Stack is full! Cannot push new bill.\n");
        return;
    }
    billStack.bills[++billStack.top] = ride;
    printf("Bill pushed into stack and saved for Admin.\n");
}

Ride *popBill()
{
    if (isBillStackEmpty())
    {
        printf("No bills to display!\n");
        return NULL;
    }
    return billStack.bills[billStack.top--];
}

float calculateFare(Graph *city, char pickup[], char drop[])
{
    int pickupIndex = getLocationIndex(city, pickup);
    int dropIndex = getLocationIndex(city, drop);

    if (pickupIndex == -1 || dropIndex == -1)
    {
        printf("Invalid pickup/drop location for fare calculation!\n");
        return 0;
    }

    int dist[MAX], visited[MAX];
    for (int i = 0; i < city->n; i++)
    {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }

    PriorityQueue pq;
    initPQ(&pq);
    push(&pq, pickupIndex, 0);
    dist[pickupIndex] = 0;

    while (!isEmpty(&pq))
    {
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

    if (dist[dropIndex] == INT_MAX)
    {
        printf("No route found between %s and %s!\n", pickup, drop);
        return 0;
    }

    float baseFare = 30.0;
    float perKmRate = 10.0;
    float totalFare = baseFare + (dist[dropIndex] * perKmRate);

    printf("\nCalculated Distance: %d km\n", dist[dropIndex]);
    printf("Fare: %.2f rupees\n", totalFare);

    return totalFare;
}
void displayAllBills()
{
    if (isBillStackEmpty())
    {
        printf("No bills in stack yet.\n");
        return;
    }

    printf("\n===== All Completed Ride Bills =====\n");
    for (int i = billStack.top; i >= 0; i--)
    {
        Ride *r = billStack.bills[i];
        printf("\nRide ID   : %d\nCustomer  : %d\nDriver    : %d\nPickup    : %s\nDrop      : %s\nDistance  : %d km\nFare      : %.2f rupees\nStatus    : %s\n",
               r->rideID, r->customerId, r->driverId, r->pickup, r->drop, r->distance, r->fare, r->status);
        printf("------------------------------------\n");
    }
}

void generateBill(RideStack *s, int rideId, int custId, int driverId, char pickup[], char drop[], int distance)
{
    Ride r;
    r.rideID = rideId;
    r.customerId = custId;
    r.driverId = driverId;
    strcpy(r.pickup, pickup);
    strcpy(r.drop, drop);
    r.distance = distance;

    // Calculate fare based on distance
    r.fare = calculateFare(city,r.pickup,r.drop);

    strcpy(r.status, "Completed");

    pushStack(s, r);
    Ride *billCopy = (Ride *)malloc(sizeof(Ride));
    *billCopy = r; 
    pushBill(billCopy);

    FILE *fp = fopen("rides.txt", "rb+");
    if (fp)
    {
        Ride temp;
        while (fread(&temp, sizeof(Ride), 1, fp))
        {
            if (temp.rideID == r.rideID)
            {
                fseek(fp, -sizeof(Ride), SEEK_CUR);
                fwrite(&r, sizeof(Ride), 1, fp);
                break;
            }
        }
        fclose(fp);
    }

    // Print bill
    printf("\n===== Ride Bill =====\n");
    printf("Ride ID   : %d\n", r.rideID);
    printf("Customer  : %d\n", r.customerId);
    printf("Driver    : %d\n", r.driverId);
    printf("Pickup    : %s\n", r.pickup);
    printf("Drop      : %s\n", r.drop);
    printf("Distance  : %d km\n", r.distance);
    printf("Fare      : %.2f rupees\n", r.fare);
    printf("=====================\n");
}
