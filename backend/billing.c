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
void loadBillsFromFile()
{
    FILE *fp = fopen("bills.txt", "rb");
    if (!fp)
        return;

    initBillStack(); // clear stack first

    Ride r;
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        Ride *billCopy = (Ride *)malloc(sizeof(Ride));
        *billCopy = r;
        pushBill(billCopy);
    }

    fclose(fp);
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

    //printf("\nCalculated Distance: %d km\n", dist[dropIndex]);
    //printf("Fare: %.2f rupees\n", totalFare);

    return totalFare;
}
// Display all bills with timestamps
void displayAllBills()
{
    FILE *fp = fopen("bills.txt", "rb");
    if (!fp)
    {
        printf("No bills found!\n");
        return;
    }

    Ride r;
    int found = 0;
    printf("\n===== All Completed Ride Bills =====\n");

    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        printf("\nRide ID   : %d\nCustomer  : %d\nDriver    : %d\nPickup    : %s\nDrop      : %s\nDistance  : %d km\nFare      : %.2f rupees\nStatus    : %s\n",
               r.rideID, r.customerId, r.driverId, r.pickup, r.drop, r.distance, r.fare, r.status);
        printf("Booking Time: ");
        printTime(r.bookingTime);
        printf("\n");
        printf("Start Time  : ");
        printTime(r.startTime);
        printf("\n");
        printf("End Time    : ");
        printTime(r.endTime);
        printf("\n");
        printf("------------------------------------\n");
        found = 1;
    }

    if (!found)
        printf("No completed ride bills found.\n");

    fclose(fp);
}

// Generate bill for a completed ride
void generateBill(RideStack *s, int rideId, int custId, int driverId, char pickup[], char drop[], int distance, float fare)
{
    Ride r;
    int found = 0;

    // Read ride from rides.txt to get correct timestamps
    FILE *fp = fopen("rides.txt", "rb");
    if (fp)
    {
        Ride temp;
        while (fread(&temp, sizeof(Ride), 1, fp))
        {
            if (temp.rideID == rideId)
            {
                r = temp; // use actual ride with correct times
                found = 1;
                break;
            }
        }
        fclose(fp);
    }

    if (!found)
    {
        printf("Ride not found!\n");
        return;
    }

    // Recalculate fare if needed
    r.fare = calculateFare(city, r.pickup, r.drop);

    // Push into in-memory stack
    pushStack(s, r);

    Ride *billCopy = (Ride *)malloc(sizeof(Ride));
    *billCopy = r;
    pushBill(billCopy);

    // Append to bills.txt only if not already present
    FILE *bfp = fopen("bills.txt", "rb");
    int duplicate = 0;
    if (bfp)
    {
        Ride temp;
        while (fread(&temp, sizeof(Ride), 1, bfp))
        {
            if (temp.rideID == rideId)
            {
                duplicate = 1;
                break;
            }
        }
        fclose(bfp);
    }

    if (!duplicate)
    {
        bfp = fopen("bills.txt", "ab");
        if (bfp)
        {
            fwrite(&r, sizeof(Ride), 1, bfp);
            fclose(bfp);
        }
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
    printf("Booking Time: ");
    printTime(r.bookingTime);
    printf("\nStart Time  : ");
    printTime(r.startTime);
    printf("\nEnd Time    : ");
    printTime(r.endTime);
    printf("\n=====================\n");
}
