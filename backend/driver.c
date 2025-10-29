#include <stdio.h>
#include <string.h>
#include "map.h"
#include "driver.h"
#include "ride.h"
#include "stack.h"
#include "billing.h"

RideStack rs;
Ride billRideData;
Graph *city;

void driverMenu(int driverId)
{
    int ch;
    do
    {
        printf("\n--- Driver Menu ---\n");
        printf("1. View Assigned Rides\n");
        printf("2. Set Current Location\n");
        printf("3. View Completed Rides\n");
        printf("4. Set Availability (Online/Offline)\n");
        printf("5. Update Profile\n");
        printf("6. Logout\n");
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
            setDriverAvailability(driverId);
            break;
        case 5:
            updateDriverDetails(driverId);
            break;
        case 6:
            printf("Logging out...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (ch != 6);
}

// This function now shows assigned rides AND lets driver Accept/Reject/Start/End
void viewAssignedRides(int driverId)
{
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

    printf("\n--- Assigned Rides ---\n");

    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.driverId == driverId &&
            (strcmp(r.status, "Waiting") == 0 ||
             strcmp(r.status, "Pending") == 0 ||
             strcmp(r.status, "Accepted") == 0 ||
             strcmp(r.status, "Ongoing") == 0))
        {
            found = 1;

            printf("\nRide ID: %d", r.rideID);
            printf("\nCustomer ID: %d", r.customerId);
            printf("\nFrom: %s", r.pickup);
            printf("\nTo: %s", r.drop);
            printf("\nStatus: %s", r.status);
            printf("\nBooking Time: ");
            printTime(r.bookingTime);
            printf("\n");

            int choice;
            if (strcmp(r.status, "Pending") == 0 || strcmp(r.status, "Waiting") == 0)
            {
                printf("\n1. Accept Ride\n2. Reject Ride\n3. Skip\nEnter choice: ");
                scanf("%d", &choice);
                getchar();

                if (choice == 1)
                {
                    strcpy(r.status, "Accepted");
                    printf("Ride Accepted!\n");
                }
                else if (choice == 2)
                {
                    // Make driver available again
                    FILE *dfp = fopen("drivers.txt", "rb+");
                    if (dfp)
                    {
                        Driver d;
                        while (fread(&d, sizeof(Driver), 1, dfp))
                        {
                            if (d.id == driverId)
                            {
                                d.available = 1;
                                fseek(dfp, -sizeof(Driver), SEEK_CUR);
                                fwrite(&d, sizeof(Driver), 1, dfp);
                                break;
                            }
                        }
                        fclose(dfp);
                    }

                    printf("Ride Rejected! Attempting reassignment...\n");
                    reassignRide(&r, city, driverId);
                }
            }
            else if (strcmp(r.status, "Accepted") == 0)
            {
                printf("\n1. Start Ride\n2. Skip\nEnter choice: ");
                scanf("%d", &choice);
                getchar();

                if (choice == 1)
                {
                    strcpy(r.status, "Ongoing");
                    r.startTime = time(NULL); // set ride start time
                    printf("Ride Started!\n");
                }
            }
            else if (strcmp(r.status, "Ongoing") == 0)
            {
                printf("\n1. End Ride\n2. Skip\nEnter choice: ");
                scanf("%d", &choice);
                getchar();

                if (choice == 1)
                {
                    strcpy(r.status, "Completed");
                    r.endTime = time(NULL); // set ride end time
                    printf("Ride Completed Successfully!\n");

                    // Make driver available again
                    FILE *dfp = fopen("drivers.txt", "rb+");
                    if (dfp)
                    {
                        Driver d;
                        while (fread(&d, sizeof(Driver), 1, dfp))
                        {
                            if (d.id == driverId)
                            {
                                d.available = 1;
                                fseek(dfp, -sizeof(Driver), SEEK_CUR);
                                fwrite(&d, sizeof(Driver), 1, dfp);
                                break;
                            }
                        }
                        fclose(dfp);
                    }

                    // Calculate distance & fare
                    int pickupIndex = getLocationIndex(city, r.pickup);
                    int dropIndex = getLocationIndex(city, r.drop);
                    if (pickupIndex != -1 && dropIndex != -1)
                    {
                        r.distance = calculateDistance(city, pickupIndex, dropIndex);
                        r.fare = calculateFare(city, r.pickup, r.drop);
                        printf("Total Distance: %d km\n", r.distance);
                        printf("Fare: %.2f rupees\n", r.fare);
                    }

                    // Update driver location
                    dfp = fopen("drivers.txt", "rb+");
                    if (dfp)
                    {
                        Driver d;
                        while (fread(&d, sizeof(Driver), 1, dfp))
                        {
                            if (d.id == driverId)
                            {
                                d.location = dropIndex;
                                d.completedRides += 1;     // increment completed rides
                                d.earnings += r.fare; // add fare to total earnings
                                fseek(dfp, -sizeof(Driver), SEEK_CUR);
                                fwrite(&d, sizeof(Driver), 1, dfp);
                                break;
                            }
                        }
                        fclose(dfp);
                    }

                    // update ride count for customer
                    FILE *cfp = fopen("customer.txt", "rb+");
                    if (cfp)
                    {
                        Customer cust;
                        while (fread(&cust, sizeof(Customer), 1, cfp))
                        {
                            if (cust.id == r.customerId)
                            {
                                cust.totalRides += 1;
                                fseek(cfp, -sizeof(Customer), SEEK_CUR);
                                fwrite(&cust, sizeof(Customer), 1, cfp);
                                break;
                            }
                        }
                        fclose(cfp);
                    }

                    FILE *bfp = fopen("bills.txt", "ab");
                    if (bfp){
                        fwrite(&r, sizeof(Ride), 1, bfp);
                        fclose(bfp);
                    }
                    // Optionally: generate bill here
                    //generateBill(&rs, r.rideID, r.customerId, r.driverId, r.pickup, r.drop, r.distance, r.fare);
                }
            }
        }

        fwrite(&r, sizeof(Ride), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    if (found)
    {
        remove("rides.txt");
        rename("temp.txt", "rides.txt");
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
            found = 1;
            printf("\nRide ID: %d", r.rideID);
            printf("\nCustomer ID: %d", r.customerId);
            printf("\nFrom: %s", r.pickup);
            printf("\nTo: %s", r.drop);
            printf("\nDistance: %d km", r.distance);
            printf("\nFare: %.2f", r.fare);
            printf("\nStatus: %s", r.status);

            // Print timestamps
            printf("\nBooking Time: "); printTime(r.bookingTime);
            printf("\nStart Time: "); printTime(r.startTime);
            printf("\nEnd Time: "); printTime(r.endTime);
            printf("\n---------------------------\n");
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

void updateDriverDetails(int driverID)
{
    FILE *fp = fopen("drivers.txt", "rb+");
    if (fp == NULL)
    {
        printf("No drivers found!\n");
        return;
    }

    Driver d;
    int found = 0;
    long pos;

    while (fread(&d, sizeof(Driver), 1, fp))
    {
        if (d.id == driverID)
        {
            found = 1;
            pos = ftell(fp) - sizeof(Driver);
            break;
        }
    }

    if (!found)
    {
        printf("Driver not found!\n");
        fclose(fp);
        return;
    }

    int choice;
    printf("\n--- Update Profile ---\n");
    printf("1. Name\n2. Vehicle\n3. Password\nEnter choice: ");
    scanf("%d", &choice);
    getchar();

    switch (choice)
    {
    case 1:
        printf("Enter new name: ");
        fgets(d.name, sizeof(d.name), stdin);
        d.name[strcspn(d.name, "\n")] = '\0';
        break;
    case 2:
        printf("Enter new vehicle: ");
        fgets(d.vehicle, sizeof(d.vehicle), stdin);
        d.vehicle[strcspn(d.vehicle, "\n")] = '\0';
        break;
    case 3:
        printf("Enter new password: ");
        scanf("%s", d.pass);
        break;
    default:
        printf("Invalid choice!\n");
        fclose(fp);
        return;
    }

    fseek(fp, pos, SEEK_SET);
    fwrite(&d, sizeof(Driver), 1, fp);
    fflush(fp);
    fclose(fp);

    printf("\nProfile updated successfully!\n");
}

void setDriverAvailability(int driverId)
{
    FILE *fp = fopen("drivers.txt", "rb+");
    if (!fp)
    {
        printf("Error opening driver file.\n");
        return;
    }

    Driver d;
    int found = 0;
    while (fread(&d, sizeof(Driver), 1, fp))
    {
        if (d.id == driverId)
        {
            found = 1;
            printf("\nCurrent status: %s\n", d.available ? "Online" : "Offline");
            printf("Set new availability (1 = Online, 0 = Offline): ");
            scanf("%d", &d.available);

            fseek(fp, -sizeof(Driver), SEEK_CUR);
            fwrite(&d, sizeof(Driver), 1, fp);
            fflush(fp);

            printf("Availability updated successfully!\n");
            break;
        }
    }

    if (!found)
        printf("Driver not found!\n");

    fclose(fp);
}
