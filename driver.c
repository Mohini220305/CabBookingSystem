#include <stdio.h>
#include <string.h>
#include "driver.h"
#include "ride.h"
#include "stack.h"
#include "map.h"
#include "billing.h"

RideStack rs;
Ride billRideData;

void driverMenu(int driverId)
{
    int ch;
    do
    {
        printf("\n--- Driver Menu ---\n");
        printf("1. View Assigned Rides\n");
        printf("2. Set Current Location\n");
        printf("3. View Completed Rides\n");
        printf("4. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &ch);
        getchar();

        switch (ch)
        {
        case 1:
            viewAssignedRides(driverId);
            break;
        case 2:
            displayMap(city);
            updateDriverLocation(driverId);
            break;
        case 3:
            viewCompletedRides(driverId);
            break;
        case 4:
            printf("Logging out...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (ch != 4);
}

// This function now shows assigned rides AND lets driver Accept/Reject/Start/End
void viewAssignedRides(int driverId)
{
    int shouldGenerateBill = 0;
    FILE *fp = fopen("rides.txt", "rb");
    FILE *temp = fopen("temp.txt", "wb");
    if (!fp || !temp)
    {
        printf("No rides found!\n");
        if (fp)
            fclose(fp);
        if (temp)
            fclose(temp);
        return;
    }

    Ride r;
    int found = 0;
    int distance = 0;
    printf("\n--- Assigned Rides ---\n");
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.driverId == driverId &&
             (strcmp(r.status, "Waiting") == 0 ||strcmp(r.status, "Pending") == 0 || strcmp(r.status, "Accepted") == 0 || strcmp(r.status, "Ongoing") == 0))
        {
            found = 1;

            printf("\nRide ID: %d", r.rideID);
            printf("\nCustomer ID: %d", r.customerId);
            printf("\nFrom: %s", r.pickup);
            printf("\nTo: %s", r.drop);
            printf("\nStatus: %s\n", r.status);

            int choice;
            if (strcmp(r.status, "Pending") == 0 || strcmp(r.status, "Waiting") == 0)
            {
                printf("\n1. Accept Ride\n2. Reject Ride\n3. Skip\nEnter choice: ");
                scanf("%d", &choice);
                if (choice == 1)
                {
                    strcpy(r.status, "Accepted");
                    printf("Ride Accepted!\n");
                }
                else if (choice == 2)
                {
                    Driver drivers[50];
                    int numDrivers = loadDrivers(drivers);
                    for (int i = 0; i < numDrivers; i++)
                    {
                        if (drivers[i].id == driverId)
                        {
                            drivers[i].available = 1;
                            break;
                        }
                    }
                    FILE *dfp = fopen("drivers.txt", "wb");
                    if (dfp)
                    {
                        fwrite(drivers, sizeof(Driver), numDrivers, dfp);
                        fclose(dfp);
                    }

                    // Reassign 
                    reassignRide(&r, city);
                }
            }
            else if (strcmp(r.status, "Accepted") == 0)
            {
                printf("\n1. Start Ride\n2. Skip\nEnter choice: ");
                scanf("%d", &choice);
                if (choice == 1)
                {
                    strcpy(r.status, "Ongoing");
                    printf("Ride Started!\n");
                }
            }
            else if (strcmp(r.status, "Ongoing") == 0)
            {
                printf("\n1. End Ride\n2. Skip\nEnter choice: ");
                scanf("%d", &choice);
                if (choice == 1)
                {
                    strcpy(r.status, "Completed");
                    printf("Ride Completed Successfully!\n");

                    // Make driver available again
                    Driver drivers[50];
                    int numDrivers = loadDrivers(drivers);
                    for (int i = 0; i < numDrivers; i++)
                    {
                        if (drivers[i].id == driverId)
                        {
                            drivers[i].available = 1;
                            break;
                        }
                    }
                    FILE *dfp = fopen("drivers.txt", "wb");
                    if (dfp)
                    {
                        fwrite(drivers, sizeof(Driver), numDrivers, dfp);
                        fclose(dfp);
                    }

                    int pickupIndex = getLocationIndex(city, r.pickup);
                    int dropIndex = getLocationIndex(city, r.drop);
                    if (pickupIndex != -1 && dropIndex != -1)
                    {
                        
                        distance = calculateDistance(city, pickupIndex, dropIndex);
                        printf("%d ", distance);
                    }

                    r.distance = distance;
                    shouldGenerateBill = 1;
                    billRideData = r;
                }
            }
        }
        r.distance = distance;
        fwrite(&r, sizeof(Ride), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    if (found)
    {
        remove("rides.txt");
        rename("temp.txt", "rides.txt");
        if (shouldGenerateBill)
        {
            generateBill(&rs, billRideData.rideID, billRideData.customerId,billRideData.driverId, billRideData.pickup,billRideData.drop, billRideData.distance);
        }
    }
    else
    {
        remove("temp.txt");
        printf("No assigned rides right now.\n");
    }
}

// Driver can update their location
void updateDriverLocation(int driverId)
{
    Driver drivers[100];
    int numDrivers = loadDrivers(drivers);

    int i;
    for (i = 0; i < numDrivers; i++)
        if (drivers[i].id == driverId)
            break;

    if (i == numDrivers)
    {
        printf("Driver not found!\n");
        return;
    }

    char locName[50];
    printf("\nEnter your current location: ");
    fgets(locName, sizeof(locName), stdin);
    locName[strcspn(locName, "\n")] = 0;

    int locIndex = getLocationIndex(city, locName);
    if (locIndex == -1)
    {
        printf("Invalid location!\n");
        return;
    }

    drivers[i].location = locIndex;

    FILE *dfp = fopen("drivers.txt", "wb");
    if (dfp)
    {
        fwrite(drivers, sizeof(Driver), numDrivers, dfp);
        fclose(dfp);
    }

    printf("Location updated successfully!\n");
}

// Show completed rides
void viewCompletedRides(int driverId)
{
    FILE *fp = fopen("rides.txt", "rb");
    if (!fp)
    {
        printf("No rides found!\n");
        return;
    }

    Ride r;
    int found = 0;
    printf("\n--- Completed Rides ---\n");

    while (fread(&r, sizeof(Ride), 1, fp))
    {
        if (r.driverId == driverId && strcmp(r.status, "Completed") == 0)
        {
            printf("\nRide ID: %d", r.rideID);
            printf("\tCustomer ID: %d", r.customerId);
            printf("\tFrom: %s", r.pickup);
            printf("\tTo: %s", r.drop);
            printf("\tDistance: %d km", r.distance);
            printf("\tFare: %.2f", r.fare);
            printf("\tStatus: %s\n", r.status);
            found = 1;
        }
    }

    if (!found)
        printf("No completed rides found.\n");

    fclose(fp);
}

int loadDrivers(Driver drivers[])
{
    FILE *fp = fopen("drivers.txt", "rb");
    if (!fp)
    {
        printf("Warning: drivers.txt not found! No drivers loaded.\n");
        return 0;
    }

    int count = 0;
    while (fread(&drivers[count], sizeof(Driver), 1, fp) == 1)
    {
        //printf("Loaded Driver: %d | %s | Location: %d | Available: %d\n",drivers[count].id, drivers[count].name, drivers[count].location, drivers[count].available);
        count++;
    }

    fclose(fp);
    //printf("Total drivers : %d\n", count);
    return count;
}

