#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "admin.h"
#include "driver.h"

void manage_driver()
{
    Driver d;
    int ch;
    FILE *fp;
    do
    {
        printf("\n---Manage Driver---\n");
        printf("1. Add Driver\n");
        printf("2. Remove Driver\n");
        printf("3. Modify Driver Details\n");
        printf("4. View Driver List\n");
        printf("5. Exit\n");
        printf("Please select option : ");
        scanf("%d", &ch);

        switch (ch)
        {
        case 1:
        {
            Driver d;
            int newId = 1;
            FILE *fp = fopen("drivers.txt", "rb");
            if (fp)
            {
                Driver temp;
                while (fread(&temp, sizeof(Driver), 1, fp))
                {
                    if (temp.id >= newId)
                        newId = temp.id + 1;
                }
                fclose(fp);
            }

            d.id = newId;

            printf("Enter Driver Name: ");
            getchar();
            scanf("%[^\n]", d.name);

            printf("Enter Vehicle Type: ");
            getchar();
            scanf("%[^\n]", d.vehicle);

            printf("Enter Phone Number: ");
            scanf("%s", d.phone);

            strcpy(d.pass, "driver123"); // default password
            d.available = 1; // Driver is free by default
            d.location = 1;  // Default location index (ISBT)
            d.mustChangePassword = 1;

            fp = fopen("drivers.txt", "ab");
            if (!fp)
            {
                printf("Error opening drivers.txt!\n");
                return;
            }
            fwrite(&d, sizeof(Driver), 1, fp);
            fclose(fp);

            printf("Driver added successfully! ID: %d | Initial Location: ISBT\n", d.id);
        }
        break;
        case 2:
        {
            int id, idFound = 0;
            printf("Please enter driver id : ");
            scanf("%d", &id);

            fp = fopen("drivers.txt", "rb");
            FILE *temp = fopen("temp.txt", "wb");
            if (fp == NULL)
            {
                printf("No drivers found!\n");
                break;
            }
            if (temp == NULL)
            {
                printf("File error!\n");
                fclose(fp);
                break;
            }

            while (fread(&d, sizeof(Driver), 1, fp) == 1)
            {
                if (d.id == id)
                {
                    idFound = 1;
                    continue;
                }
                fwrite(&d, sizeof(Driver), 1, temp);
            }
            fclose(fp);
            fclose(temp);

            if (idFound)
            {
                remove("drivers.txt");
                rename("temp.txt", "drivers.txt");
                printf("Driver removed successfully!\n");
            }
            else
            {
                remove("temp.txt");
                printf("Driver ID not found!\n");
            }
            break;
        }

        case 3:
        {
            int id, found = 0;
            printf("Enter Driver ID to modify: ");
            scanf("%d", &id);

            fp = fopen("drivers.txt", "rb+");
            if (fp == NULL)
            {
                printf("No drivers found!\n");
                break;
            }

            while (fread(&d, sizeof(Driver), 1, fp) == 1)
            {
                if (d.id == id)
                {
                    found = 1;
                    while (getchar() != '\n');
                    printf("Enter new Name: ");
                    fgets(d.name, sizeof(d.name), stdin);
                    d.name[strcspn(d.name, "\n")] = '\0';

                    printf("Enter new Phone No.: ");
                    fgets(d.phone, sizeof(d.phone), stdin);
                    d.phone[strcspn(d.phone, "\n")] = '\0';

                    printf("Enter new Vehicle: ");
                    fgets(d.vehicle, sizeof(d.vehicle), stdin);
                    d.vehicle[strcspn(d.vehicle, "\n")] = '\0';

                    fseek(fp, -sizeof(Driver), SEEK_CUR);
                    fwrite(&d, sizeof(Driver), 1, fp);
                    printf("Driver details updated successfully!\n");
                    break;
                }
            }
            if (!found)
                printf("Driver ID not found!\n");
            fclose(fp);
            break;
        }

        case 4:
            fp = fopen("drivers.txt", "rb");
            if (fp == NULL)
            {
                printf("No drivers found!\n");
                break;
            }

            printf("\n\t=== Driver List ===\n");
            while (fread(&d, sizeof(Driver), 1, fp) == 1)
            {
                printf("ID: %d\tName: %s\tPhone: %s\tVehicle: %s\n",
                       d.id, d.name, d.phone, d.vehicle);
            }
            fclose(fp);
            break;

        case 5:
            printf("Exiting Driver Management...\n");
            break;

        default:
            printf("Invalid option selected.\n");
            break;
        }
    } while (ch != 5);
}
