#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "login.h"

#define DRIVER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt"
#define CUSTOMER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt"

/* ------------------------- Utility: Extract value ------------------------- */
char *getParam(char *query, const char *key, char *value)
{
    char *pos = strstr(query, key);
    if (pos)
    {
        pos += strlen(key) + 1;
        sscanf(pos, "%[^&]", value);
        return value;
    }
    strcpy(value, "");
    return value;
}

/* ------------------------- ADMIN LOGIN ------------------------- */
int adminLogin(char *query)
{
    char username[50], password[50];
    getParam(query, "username", username);
    getParam(query, "password", password);

    printf("Content-type:text/html\n\n");

    if (strcmp(username, "Admin") == 0 && strcmp(password, "admin123") == 0)
    {
        printf("<h2>Admin Login Successful!</h2>");
        printf("<meta http-equiv='refresh' content='1;url=/CAB_BOOKING/frontend/admin_dashboard.html'>");
        return 1;
    }
    else
    {
        printf("<h3>Invalid Admin Credentials!</h3>");
        printf("<a href='/CAB_BOOKING/frontend/admin_login.html'>Try Again</a>");
        return 0;
    }
}

/* ------------------------- DRIVER LOGIN WITH PASSWORD CHANGE ------------------------- */
int driverLogin(char *query)
{
    char idStr[20] = "", password[50] = "";
    int driverID;
    FILE *fp;
    Driver d;

    // Get parameters from query
    getParam(query, "driverID", idStr);
    getParam(query, "password", password);

    driverID = atoi(idStr);

    printf("Content-type:text/html\n\n");

    if (driverID <= 0)
    {
        printf("<h3>Invalid Driver ID. Please try again.</h3>");
        printf("<a href='/CAB_BOOKING/frontend/driver_login.html'>Back to Login</a>");
        return 0;
    }

    fp = fopen(DRIVER_FILE, "rb");
    if (!fp)
    {
        printf("<h3>Error: Driver file not found!</h3>");
        return 0;
    }

    int found = 0;
    while (fread(&d, sizeof(Driver), 1, fp) == 1)
    {
        if (d.id == driverID)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("<h3>Driver ID not found.</h3>");
        printf("<a href='/CAB_BOOKING/frontend/driver_login.html'>Back to Login</a>");
        fclose(fp);
        return 0;
    }

    /* --------------------------------------------------------------------
       ✅ Normal login check (password verification only)
    -------------------------------------------------------------------- */
    if (strcmp(password, d.pass) == 0)
    {
        fclose(fp);

        // --- If password change required (first-time login) ---
        if (d.mustChangePassword)
        {
            printf("<!DOCTYPE html><html><head><title>Password Change Required</title>");
            printf("<style>");
            printf("body { font-family: Arial, sans-serif; background: #f0f2f5; text-align: center; padding-top: 100px; }");
            printf(".box { background: white; padding: 30px; border-radius: 12px; display: inline-block; box-shadow: 0 0 10px rgba(0,0,0,0.1); width: 400px; }");
            printf(".btn { background-color: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; text-decoration:none; }");
            printf(".btn:hover { background-color: #0056b3; }");
            printf("</style></head><body>");
            printf("<div class='box'>");
            printf("<h2>Hello, %s!</h2>", d.name);
            printf("<p>You are using a temporary password.<br>Please change it from your dashboard.</p>");
            printf("<a href='/CAB_BOOKING/frontend/driver_dashboard.html?driverId=%d' class='btn'>Go to Dashboard</a>", driverID);
            printf("</div></body></html>");
            return driverID;
        }

        // --- Normal successful login ---
        printf("<!DOCTYPE html><html><head>");
        printf("<meta http-equiv='refresh' content='1;url=/CAB_BOOKING/frontend/driver_dashboard.html?driverId=%d'>", driverID);
        printf("</head><body>");
        printf("<h2>Login Successful!</h2>");
        printf("<p>Redirecting to your dashboard...</p>");
        printf("</body></html>");
        return driverID;
    }
    else
    {
        printf("<h3>Incorrect Password!</h3>");
        printf("<a href='/CAB_BOOKING/frontend/driver_login.html'>Back to Login</a>");
        fclose(fp);
        return 0;
    }
}

/* ------------------------- CUSTOMER LOGIN ------------------------- */
int customerLogin(char *query)
{
    char idStr[20], password[50];
    int customerID;
    FILE *fp;
    Customer c;

    getParam(query, "customerID", idStr);
    getParam(query, "password", password);
    customerID = atoi(idStr);

    printf("Content-type:text/html\n\n");

    fp = fopen(CUSTOMER_FILE, "rb");
    if (!fp)
    {
        printf("<h3>No customers registered yet!</h3>");
        return 0;
    }

    while (fread(&c, sizeof(Customer), 1, fp) == 1)
    {
        if (c.id == customerID && strcmp(password, c.password) == 0)
        {
            if (c.is_blocked)
            {
                printf("<h3>Your account is blocked. Please contact support.</h3>");
                fclose(fp);
                return 0;
            }

            printf("<h2>Customer Login Successful!</h2>");

            // ✅ Redirect with customerId
            printf("<meta http-equiv='refresh' content='1;url=/CAB_BOOKING/frontend/customer_dashboard.html?custId=%d'>", customerID);

            fclose(fp);
            return customerID;
        }
    }

    printf("<h3>Invalid Customer Credentials!</h3>");
    fclose(fp);
    return 0;
}

/* ------------------------- CUSTOMER REGISTRATION ------------------------- */
void registerCustomer(char *query)
{
    char name[50], phone[20], password[50];
    Customer c, temp;

    FILE *fp = fopen(CUSTOMER_FILE, "ab+");
    if (!fp)
    {
        printf("Content-type:text/html\n\n");
        printf("<h3>Error opening customer file!</h3>");
        return;
    }

    getParam(query, "name", name);
    getParam(query, "phone", phone);
    getParam(query, "password", password);

    c.id = 1;
    fseek(fp, 0, SEEK_SET);
    while (fread(&temp, sizeof(Customer), 1, fp) == 1)
    {
        if (temp.id >= c.id)
            c.id = temp.id + 1;
    }

    strcpy(c.name, name);
    strcpy(c.phone, phone);
    strcpy(c.password, password);
    c.is_blocked = 0;
    c.totalRides = 0;

    fseek(fp, 0, SEEK_END);
    fwrite(&c, sizeof(Customer), 1, fp);
    fclose(fp);

    printf("Content-type:text/html\n\n");
    printf("<h2>Registration Successful!</h2>");
    printf("<p>Your Customer ID is: %d</p>", c.id);
    printf("<meta http-equiv='refresh' content='2;url=/CAB_BOOKING/frontend/customer_login.html'>");
}

/* ------------------------- MAIN ------------------------- */
int main()
{
    char *query = getenv("QUERY_STRING");
    if (!query)
    {
        printf("Content-type:text/html\n\n");
        printf("<h3>No data received!</h3>");
        return 0;
    }

    char type[20];
    getParam(query, "type", type);

    if (strcmp(type, "admin") == 0)
        adminLogin(query);
    else if (strcmp(type, "driver") == 0)
        driverLogin(query);
    else if (strcmp(type, "customer") == 0)
        customerLogin(query);
    else if (strcmp(type, "register") == 0)
        registerCustomer(query);
    else
    {
        printf("Content-type:text/html\n\n");
        printf("<h3>Invalid request type!</h3>");
    }

    return 0;
}
