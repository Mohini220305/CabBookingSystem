#include "admin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "customer.h"
#include "driver.h"
#include "ride.h"
#include "stack.h"
#include "billing.h"

RideStack rs;

void adminMenu()
{
    int choice;
    do
    {
        system("cls");
        printf("=== Admin Menu ===\n");
        printf("1. Manage Customers\n");
        printf("2. Manage Drivers\n");
        printf("3. View Ride History\n");
        printf("4. View Reports\n");
        printf("5. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            manage_customer();
            break;
        case 2:
            manage_driver();
            break;
        case 3:
            viewAllRides(&rs);
            break;
        case 4:
            displayAllBills();
            break;
        case 5:
            printf("Logging out...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
        printf("Press Enter to continue...");
        getchar();
        getchar();
    } while (choice != 5);
}

void viewAllRides()
{
    FILE *fp = fopen("rides.txt", "rb");
    if (fp == NULL)
    {
        printf("No rides found!\n");
        return;
    }

    Ride r;
    int found = 0;
    printf("\n--- All Rides ---\n");
    while (fread(&r, sizeof(Ride), 1, fp))
    {
        printf("Ride %d | Cust %d | Driver %d | %s -> %s | Fare: %.2f | Status: %s\n",
               r.rideID, r.customerId, r.driverId,
               r.pickup, r.drop, r.fare, r.status);
        found = 1;
    }

    if (!found)
        printf("No rides recorded yet.\n");

    fclose(fp);
}
