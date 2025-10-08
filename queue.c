#include "ride.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "admin.h"
#include "customer.h"
#include "driver.h"
#include "map.h"

int rideID = 1;
Ride *front = NULL;
Ride *rear = NULL;

void enqueueRide(Ride *newRide){
    if (rear == NULL){
        front = rear = newRide;
    }
    else{
        rear->next = newRide;
        rear = newRide;
    }
    newRide->next = NULL;
}

void saveRideToFile(Ride *r){
    FILE *fp = fopen("rides.txt", "ab");
    if (fp == NULL){
        printf("Error opening rides file!\n");
        return;
    }
    fwrite(r, sizeof(Ride), 1, fp);
    fclose(fp);
}

void loadRidesFromFile(){
    FILE *fp = fopen("rides.txt", "rb");
    if (fp == NULL)
        return;

    Ride temp;
    while (fread(&temp, sizeof(Ride), 1, fp)){
        Ride *newRide = (Ride *)malloc(sizeof(Ride));
        *newRide = temp;
        newRide->next = NULL;
        enqueueRide(newRide);

        if (temp.rideID >= rideID)
            rideID = temp.rideID + 1;
    }
    fclose(fp);
}

// Book ride
Ride *bookRide(Graph *city, int customerId, char pickup[], char drop[])
{
   
    Driver drivers[20];
    int numDrivers = loadDrivers(drivers);

    Ride *newRide = (Ride *)malloc(sizeof(Ride));
    newRide->rideID = rideID++;
    newRide->customerId = customerId;
    newRide->driverId = -1;
    strcpy(newRide->pickup, pickup);
    strcpy(newRide->drop, drop);
    newRide->distance = 0;
    newRide->fare = 0;
    strcpy(newRide->status, "Waiting");
    newRide->next = NULL;

    int pickupLoc = getLocationIndex(city, pickup);
    int dropLoc = getLocationIndex(city, drop);

    if (pickupLoc == -1 || dropLoc == -1)
    {
        printf("Invalid pickup or drop location!\n");
        strcpy(newRide->status, "Invalid");
    }
    else
    {
        int driverId = allocateDriver(city, drivers, numDrivers, pickupLoc);
        if (driverId != -1)
        {
            newRide->driverId = driverId;
            strcpy(newRide->status, "Pending");
            printf("Nearest driver found: %d\n", driverId);
        }
        else
        {
            strcpy(newRide->status, "Waiting");
            printf("No drivers available! Ride is in waiting queue.\n");
        }
    }

    enqueueRide(newRide);
    saveRideToFile(newRide);

    if (newRide->driverId != -1)
    {
        FILE *dfp = fopen("drivers.txt", "rb+");
        if (dfp)
        {
            Driver d;
            while (fread(&d, sizeof(Driver), 1, dfp))
            {
                if (d.id == newRide->driverId)
                {
                    d.available = 0;
                    fseek(dfp, -sizeof(Driver), SEEK_CUR);
                    fwrite(&d, sizeof(Driver), 1, dfp);
                    break;
                }
            }
            fclose(dfp);
        }
    }

    printf("Ride booked! \nRide ID: %d | Status: %s\n", newRide->rideID, newRide->status);
    return newRide;
}

// Cancel Ride
void cancelRide(int custId)
{
    FILE *fp = fopen("rides.txt", "rb");
    FILE *temp = fopen("temp.txt", "wb");

    if (!fp || !temp)
    {
        printf("Error opening files!\n");
        return;
    }

    int rideId;

    printf("Enter Ride ID to cancel: ");
    scanf("%d", &rideId);

    Ride r;
    int found = 0;

    while (fread(&r, sizeof(Ride), 1, fp))
    {
        if (r.rideID == rideId && r.customerId == custId)
        {
            found = 1;
            if (strcmp(r.status, "Completed") == 0)
            {
                printf("Ride already completed! Cannot cancel.\n");
            }
            else if (strcmp(r.status, "Cancelled") == 0)
            {
                printf("Ride is already cancelled.\n");
            }
            else
            {
                strcpy(r.status, "Cancelled");

                if (r.driverId != -1)
                {
                    FILE *dfp = fopen("drivers.txt", "rb+");
                    if (dfp != NULL)
                    {
                        Driver d;
                        while (fread(&d, sizeof(Driver), 1, dfp))
                        {
                            if (d.id == r.driverId)
                            {
                                d.available = 1;
                                fseek(dfp, -sizeof(Driver), SEEK_CUR);
                                fwrite(&d, sizeof(Driver), 1, dfp);
                                break;
                            }
                        }
                        fclose(dfp);
                    }
                }

                r.driverId = -1;
                printf("Ride %d cancelled successfully!\n", rideId);
            }
        }
        fwrite(&r, sizeof(Ride), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    remove("rides.txt");
    rename("temp.txt", "rides.txt");

    if (!found)
        printf("No ride found with ID %d for this customer.\n", rideId);
        
    getchar();
}
