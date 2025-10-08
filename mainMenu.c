#include <stdio.h>
#include <stdlib.h>
#include "admin.h"
#include "customer.h"
#include "driver.h"
#include "ride.h"
#include "login.h"
#include "map.h"
#include "billing.h"

Graph *city;

int main()
{
    city = initDehradunMap();
    loadRidesFromFile();
    initBillStack();

    int role, success;
    printf("\t=========================================\n");
    printf("\t        Welcome to Cabify-C \n");
    printf("\t    C Language Powered Cab Booking\n");
    printf("\t=========================================\n\n\n");

    do
    {
        printf("Please select your role : \n");
        printf("1. Admin\n");
        printf("2. Customer\n");
        printf("3. Driver\n");
        printf("4. Exit\n\n");

        printf("Enter choice: ");
        scanf("%d", &role);

        switch (role)
        {
        case 1: 
            success = adminLogin();
            if (success)
                adminMenu();
            else
            {
                printf("Invalid credentials! Try again. \n");
            }
            break;
        case 2:
            printf("\n1. Login\n2. Register\nEnter choice: ");
            int ch;
            scanf("%d", &ch);

            if (ch == 1)
            {
                success = customerLogin();
                if (success > 0) {
                    customerMenu(city, success);
                }
                else{
                    printf("Login failed! Please try again.\n");
                }
            }
            else{
                registerCustomer();
            }
            break;

        case 3:
            success = driverLogin();
            if (success)
            {
                driverMenu(success);
            }
            else
            {
                printf("Invalid credentials! Try again. \n");
            }
            break;

        case 4:
            exit(0);

        default:
            printf("Invalid choice! Try again.\n");
        }
    } while (role != 4);
    return 0;
}
