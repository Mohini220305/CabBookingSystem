#include <stdio.h>
#include <string.h>
#include "login.h"

int adminLogin()
{
    char adminName[50];
    char password[50];

    printf("Enter your username : ");
    scanf("%s", adminName);
    printf("Enter your password : ");
    scanf("%s", password);

    if (strcmp(adminName, "Admin") == 0 && strcmp(password, "admin123") == 0)
    {
        printf("Login Successful! \n");
        return 1;
    }
    return 0;
}

int driverLogin(){
    int driverID;
    char password[50];
    FILE *fp;
    Driver d;
    printf("Enter your ID: ");
    if (scanf("%d", &driverID) != 1){
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');
    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';
    fp = fopen("drivers.txt", "rb+");
    if (fp == NULL){
        printf("No drivers found!\n");
        return 0;
    }

    int found = 0;
    long pos = 0;
    while (fread(&d, sizeof(Driver), 1, fp) == 1){
        if (d.id == driverID){
            found = 1;
            pos = ftell(fp) - sizeof(Driver);
            break;
        }
    }

    if (!found){
        printf("Driver ID not found.\n");
        fclose(fp);
        return 0;
    }
    if (strcmp(password, d.pass) == 0){
        printf("Login Successful! Welcome, %s\n", d.name);
        if (d.mustChangePassword){
            char newPass[50];
            printf("You are using a temporary password. Please set a new password: ");
            fgets(newPass, sizeof(newPass), stdin);
            newPass[strcspn(newPass, "\n")] = '\0';

            strncpy(d.pass, newPass, sizeof(d.pass) - 1);
            d.pass[sizeof(d.pass) - 1] = '\0';
            d.mustChangePassword = 0;

            fseek(fp, pos, SEEK_SET);
            fwrite(&d, sizeof(Driver), 1, fp);
            fflush(fp);

            printf("Password changed successfully. Please login again.\n");
            fclose(fp);
            return driverLogin();
        }
        fclose(fp);
        return driverID;
    }
    else{
        printf("Incorrect password.\n");
        fclose(fp);
        return 0;
    }
}

int customerLogin(){
    int customerID;
    char password[50];
    FILE *fp;
    Customer c;

    printf("Enter your ID : ");
    scanf("%d", &customerID);
    printf("Enter your password : ");
    scanf("%s", password);

    fp = fopen("customer.txt", "rb");
    if (fp == NULL){
        printf("No customers registered yet!\n");
        return 0;
    }

    while (fread(&c, sizeof(Customer), 1, fp)){
        if (c.id == customerID && strcmp(password, c.password) == 0){
            if (c.is_blocked){
                printf("Your account is blocked. Please contact support.\n");
                fclose(fp);
                return 0;
            }
            fclose(fp);
            printf("Login Successful! \n");
            return customerID;
        }
    }

    fclose(fp);
    return 0;
}

void registerCustomer(){
    FILE *fp = fopen("customer.txt", "ab+");
    if (fp == NULL){
        printf("Error opening file!\n");
        return;
    }

    Customer c;
    c.id = 1; 
    Customer temp;
    fseek(fp, 0, SEEK_SET);
    while (fread(&temp, sizeof(Customer), 1, fp) == 1){
        if (temp.id >= c.id)
            c.id = temp.id + 1;
    }

    printf("\nYour Customer ID will be: %d\n", c.id);
    getchar();
    printf("Enter Name: ");
    scanf("%[^\n]", c.name);
    getchar();

    printf("Enter Phone: ");
    scanf("%s", c.phone);

    printf("Create Password: ");
    scanf("%s", c.password);

    c.is_blocked = 0;

    fseek(fp, 0, SEEK_END);
    fwrite(&c, sizeof(Customer), 1, fp);
    fclose(fp);

    printf("\nRegistration successful! Your Customer ID is %d. You can now login.\n", c.id);
    int success = customerLogin();
    if (success != -1)
        customerMenu(city, success);
}

