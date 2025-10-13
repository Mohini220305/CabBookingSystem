#include <stdio.h>
#include <string.h>
#include "customer.h"
#include "stack.h"

RideStack rs;
Ride billRideData;
Graph *city;

void customerMenu(Graph *city, int custId)
{
    int ch;
    do
    {
        printf("\n--- Customer Menu ---\n");
        printf("1. Book a Ride\n");
        printf("2. View Ride History\n");
        printf("3. View Ride Status\n");
        printf("4. Cancel Ride\n");
        printf("5. Logout\n");
        printf("Please select an option: ");
        scanf("%d", &ch);
        while (getchar() != '\n');

        switch (ch)
        {
        case 1: // Book a Ride
        {
            displayMap(city);
            char pickUp[50], drop[50];

            printf("\nEnter pickup location: ");
            fgets(pickUp, sizeof(pickUp), stdin);
            pickUp[strcspn(pickUp, "\n")] = '\0';

            printf("Enter drop location: ");
            fgets(drop, sizeof(drop), stdin);
            drop[strcspn(drop, "\n")] = '\0';

            Ride *r = bookRide(city, custId, pickUp, drop);
            break;
        }

        case 2: // View Ride History
        {
            showCustomerHistory(custId);
            break;
        }

        case 3: // View Ride Status
        {
            viewRideStatus(custId);
            break;
        }

        case 4: // Cancel Ride
        {
            cancelRide(custId);
            break;
        }

        case 5: // Logout
            printf("\nLogging out...\n");
            break;

        default:
            printf("Invalid choice! Try again.\n");
            break;
        }

        printf("\nPress Enter to continue...");
        getchar();

    } while (ch != 5);
}

// Show all rides of a customer with timestamps
void showCustomerHistory(int custId)
{
    FILE *fp = fopen("rides.txt", "rb");
    if (fp == NULL)
    {
        printf("No rides found!\n");
        return;
    }

    Ride r;
    int found = 0;
    printf("\n--- Ride History ---\n");

    while (fread(&r, sizeof(Ride), 1, fp))
    {
        if (r.customerId == custId)
        {
            printf("\nRide ID: %d", r.rideID);
            printf("\nPickup: %s", r.pickup);
            printf("\nDrop: %s", r.drop);
            printf("\nStatus: %s", r.status);
            if (strcmp(r.status, "Accepted") == 0 || strcmp(r.status, "Ongoing") == 0)
                printf(" (Driver ID: %d)", r.driverId);

            printf("\nBooking Time: ");
            printTime(r.bookingTime);
            printf("\nStart Time  : ");
            printTime(r.startTime);
            printf("\nEnd Time    : ");
            printTime(r.endTime);
            printf("\n---------------------------\n");

            found = 1;
        }
    }

    if (!found)
        printf("No rides found for customer %d\n", custId);

    fclose(fp);
    printf("\n");
}

// View current ride status for a customer with timestamps and bill option
void viewRideStatus(int custId)
{
    FILE *fp = fopen("rides.txt", "rb");
    if (fp == NULL)
    {
        printf("No rides found!\n");
        return;
    }

    Ride r;
    int found = 0;
    printf("\n\t--- Your Rides ---\n");

    while (fread(&r, sizeof(Ride), 1, fp))
    {
        if (r.customerId == custId)
        {
            printf("\nRide ID: %d", r.rideID);
            printf("\tPickup: %s", r.pickup);
            printf("\tDrop: %s", r.drop);
            printf("\tStatus: %s", r.status);
            if (strcmp(r.status, "Accepted") == 0 || strcmp(r.status, "Ongoing") == 0)
                printf(" (Driver ID: %d)", r.driverId);

            printf("\nBooking Time: ");
            printTime(r.bookingTime);
            printf("\nStart Time  : ");
            printTime(r.startTime);
            printf("\nEnd Time    : ");
            printTime(r.endTime);

            // Only allow viewing bill if ride is completed
            if (strcmp(r.status, "Completed") == 0)
            {
                printf("\nDo you want to view/download the bill? (1-Yes / 0-No): ");
                int choice;
                scanf("%d", &choice);
                while (getchar() != '\n')
                    ;

                if (choice == 1)
                {
                    // Call generateBill with ride info
                    // It will read correct timestamps from rides.txt
                    generateBill(&rs, r.rideID, r.customerId, r.driverId, r.pickup, r.drop, r.distance, r.fare);
                }
            }

            printf("\n");
            found = 1;
        }
    }

    if (!found)
        printf("You have no rides currently.\n");

    fclose(fp);
}
