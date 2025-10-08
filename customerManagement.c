#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "admin.h"
#include "driver.h"
#include "customer.h"

void manage_customer()
{
    Customer c;
    int ch;
    FILE *fp;
    do
    {
        printf("\n---Manage Customers---\n");
        printf("1. View All Registered Customer List\n");
        printf("2. Remove Customer\n");
        printf("3. Block/Unblock Customer\n");
        printf("4. Exit\n");
        printf("Please select option : ");
        scanf("%d", &ch);
        while (getchar() != '\n');
        switch (ch)
        {
        case 1:
            fp = fopen("customer.txt", "rb");
            if (fp == NULL)
            {
                printf("No customers found!\n");
                break;
            }

            printf("\n=== Registered Customer List ===\n");
            while (fread(&c, sizeof(Customer), 1, fp) == 1)
            {
                printf("ID: %d\tName: %s\tPhone: %s\tStatus: %s\n",
                       c.id, c.name, c.phone, c.is_blocked ? "Blocked" : "Active");
            }
            fclose(fp);
            break;

        case 2:
        {
            int id, idFound = 0;
            printf("Please enter customer id : ");
            scanf("%d", &id);

            fp = fopen("customer.txt", "rb");
            FILE *temp = fopen("temp.txt", "wb");
            if (fp == NULL)
            {
                printf("No customers found!\n");
                break;
            }
            if (temp == NULL)
            {
                printf("File error!\n");
                fclose(fp);
                break;
            }

            while (fread(&c, sizeof(Customer), 1, fp) == 1)
            {
                if (c.id == id)
                {
                    idFound = 1;
                    continue;
                }
                fwrite(&c, sizeof(Customer), 1, temp);
            }
            fclose(fp);
            fclose(temp);

            if (idFound)
            {
                remove("customer.txt");
                rename("temp.txt", "customer.txt");
                printf("Customer removed successfully!\n");
            }
            else
            {
                remove("temp.txt");
                printf("Customer ID not found!\n");
            }
            break;
        }

        case 3:
        {
            int id, found = 0;
            printf("Enter Customer ID : ");
            scanf("%d", &id);

            fp = fopen("customer.txt", "rb+");
            if (fp == NULL)
            {
                printf("No customers found!\n");
                break;
            }

            while (fread(&c, sizeof(Customer), 1, fp) == 1)
            {
                if (c.id == id)
                {
                    found = 1;
                    c.is_blocked = !c.is_blocked;
                    fseek(fp, -sizeof(Customer), SEEK_CUR);
                    fwrite(&c, sizeof(Customer), 1, fp);
                    printf("Customer %s successfully.\n", c.is_blocked ? "blocked" : "unblocked");
                    break;
                }
            }
            if (!found)
                printf("Customer ID not found!\n");
            fclose(fp);
            break;
        }

        case 4:
            printf("Exiting Customer Management...\n");
            break;

        default:
            printf("Invalid option selected.\n");
            break;
        }
    } while (ch != 4);
}
