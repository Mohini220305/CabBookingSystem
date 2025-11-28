#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "customer.h"
#include "ride.h"
#include "stack.h"
#include "map.h"
#include "ratings.h"
#include "billing.h"

#define CUSTOMER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt"
#define RIDES_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt"
#define DRIVERS_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt"
#define TEMP_RIDES_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/temp_rides.txt"

extern Graph *city;

// ---------------- Utility: URL Decode + GET param ----------------
static void urlDecode(char *dst, const char *src)
{
    while (*src)
    {
        if (*src == '%' && isxdigit((unsigned char)src[1]) && isxdigit((unsigned char)src[2]))
        {
            char hex[3] = {src[1], src[2], 0};
            *dst++ = (char)strtol(hex, NULL, 16);
            src += 3;
        }
        else if (*src == '+')
        {
            *dst++ = ' ';
            src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

char *getParam(const char *query, const char *key, char *value)
{
    value[0] = '\0';
    if (!query || !key)
        return value;

    char pattern[128];
    snprintf(pattern, sizeof(pattern), "%s=", key);
    const char *pos = strstr(query, pattern);
    if (!pos)
        return value;

    pos += strlen(pattern);
    char raw[1024];
    int i = 0;
    while (*pos && *pos != '&' && i < (int)sizeof(raw) - 1)
        raw[i++] = *pos++;
    raw[i] = '\0';
    urlDecode(value, raw);
    return value;
}

// ---------------- HTML Output ----------------
void printHeader(const char *title)
{
    printf("Content-type:text/html\n\n");
    printf("<html><head><title>%s</title>", title);
    printf("<link rel='stylesheet' href='../styles/customer.css'>");
    printf("</head><body>");
}
void printFooter() { printf("</body></html>"); }

// ---------------- File Handling ----------------
int loadAllCustomers(Customer **out)
{
    FILE *fp = fopen(CUSTOMER_FILE, "rb");
    if (!fp)
    {
        *out = NULL;
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    rewind(fp);
    int count = sz / sizeof(Customer);
    if (count <= 0)
    {
        fclose(fp);
        *out = NULL;
        return 0;
    }
    Customer *arr = malloc(sizeof(Customer) * count);
    fread(arr, sizeof(Customer), count, fp);
    fclose(fp);
    *out = arr;
    return count;
}

int writeAllCustomers(Customer *arr, int count)
{
    FILE *fp = fopen(CUSTOMER_FILE, "wb");
    if (!fp)
        return 0;
    fwrite(arr, sizeof(Customer), count, fp);
    fclose(fp);
    return 1;
}

int loadAllRides(Ride **out)
{
    FILE *fp = fopen(RIDES_FILE, "rb");
    if (!fp)
    {
        *out = NULL;
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    rewind(fp);
    int count = sz / sizeof(Ride);
    if (count <= 0)
    {
        fclose(fp);
        *out = NULL;
        return 0;
    }
    Ride *arr = malloc(sizeof(Ride) * count);
    fread(arr, sizeof(Ride), count, fp);
    fclose(fp);
    *out = arr;
    return count;
}

// ---------------- Ride Queue ----------------
int rideID = 1;
Ride *front = NULL, *rear = NULL;

// Initialize rideID based on file data
void initRideID()
{
    FILE *fp = fopen(RIDES_FILE, "rb");
    if (!fp)
        return;
    Ride r;
    while (fread(&r, sizeof(Ride), 1, fp))
        rideID = r.rideID + 1;
    fclose(fp);
}

void enqueueRide(Ride *newRide)
{
    newRide->next = NULL;
    if (!rear)
        front = rear = newRide;
    else
    {
        rear->next = newRide;
        rear = newRide;
    }
}

void saveRideToFile(Ride *r)
{
    FILE *fp = fopen(RIDES_FILE, "ab");
    if (!fp)
        return;
    fwrite(r, sizeof(Ride), 1, fp);
    fclose(fp);
}

void printTime(time_t t)
{
    if (t == 0)
    {
        printf("N/A");
        return;
    }
    char buf[80];
    struct tm *tm_info = localtime(&t);
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s", buf);
}

// ---------------- Customer Dashboard ----------------
void showCustomerDashboard(Customer c)
{
    printf("<h2>Welcome, %s (Customer #%d)</h2>", c.name, c.id);
    printf("<p>Phone: %s | Total Rides: %d | Status: %s</p>",
           c.phone, c.totalRides, c.is_blocked ? "Blocked" : "Active");
}

// ---------------- Book Ride ----------------
void bookRideForm(int custId)
{
    printf("<h3>Book a Ride</h3>");
    printf("<form action='customer_dashboard.cgi' method='GET'>");
    printf("<input type='hidden' name='action' value='bookRide'>");
    printf("<input type='hidden' name='custId' value='%d'>", custId);
    printf("Pickup Location: <input type='text' name='pickup' required><br><br>");
    printf("Drop Location: <input type='text' name='drop' required><br><br>");
    printf("<input type='submit' value='Book Ride'>");
    printf("</form>");
    printf("<a href='/cgi-bin/display_map.cgi' target='_blank' style='color:#3498db;text-decoration:none;'>");
    printf("🗺️ View City Map</a>");
}

Ride *bookRide(Graph *city, int customerId, char pickup[], char drop[])
{
    if (!city)
        city = initDehradunMap();

    initRideID();
    Driver drivers[20];
    int numDrivers = loadDrivers(drivers);

    Ride *newRide = malloc(sizeof(Ride));
    newRide->rideID = rideID++;
    newRide->customerId = customerId;
    newRide->driverId = -1;
    strcpy(newRide->pickup, pickup);
    strcpy(newRide->drop, drop);
    newRide->distance = 0;
    newRide->fare = 0;
    strcpy(newRide->status, "Waiting");
    newRide->next = NULL;

    newRide->bookingTime = time(NULL);
    newRide->cancellationTime = 0;
    newRide->startTime = 0;
    newRide->endTime = 0;

    int pickupLoc = getLocationIndex(city, pickup);
    int dropLoc = getLocationIndex(city, drop);

    if (pickupLoc == -1 || dropLoc == -1)
        strcpy(newRide->status, "Invalid");
    else
    {
        int driverId = allocateDriver(city, drivers, numDrivers, pickupLoc, pickup, drop,-1);
        if (driverId != -1)
        {
            newRide->driverId = driverId;
            strcpy(newRide->status, "Pending");

            FILE *dfp = fopen(DRIVERS_FILE, "rb+");
            if (dfp)
            {
                Driver d;
                while (fread(&d, sizeof(Driver), 1, dfp))
                {
                    if (d.id == driverId)
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
    }

    enqueueRide(newRide);
    saveRideToFile(newRide);

    printf("<h3>Ride Booked!</h3>");
    printf("<table border='1'><tr><th>Ride ID</th><th>Pickup</th><th>Drop</th><th>Status</th><th>Booking Time</th></tr>");
    printf("<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>",
           newRide->rideID, newRide->pickup, newRide->drop, newRide->status);
    printTime(newRide->bookingTime);
    printf("</td></tr></table>");
    printf("<br><a href='customer_dashboard.cgi?action=dashboard&custId=%d'>Return to Dashboard</a>", customerId);

    return newRide;
}

// ---------------- Cancel Ride ----------------
void cancelRideForm(int custId)
{
    printf("<h3>Cancel Ride</h3>");
    printf("<form action='customer_dashboard.cgi' method='GET'>");
    printf("<input type='hidden' name='action' value='cancelRide'>");
    printf("<input type='hidden' name='custId' value='%d'>", custId);
    printf("Ride ID to Cancel: <input type='number' name='rideId' required><br><br>");
    printf("<input type='submit' value='Cancel Ride'>");
    printf("</form>");
}

void cancelRide(int custId, int rideId)
{
    FILE *fp = fopen(RIDES_FILE, "rb");
    FILE *temp = fopen(TEMP_RIDES_FILE, "wb");
    if (!fp || !temp)
    {
        printf("<p>Error opening rides file!</p>");
        return;
    }

    Ride r;
    int found = 0;
    while (fread(&r, sizeof(Ride), 1, fp))
    {
        if (r.rideID == rideId && r.customerId == custId)
        {
            found = 1;
            if (strcmp(r.status, "Completed") != 0 && strcmp(r.status, "Cancelled") != 0)
            {
                strcpy(r.status, "Cancelled");
                r.cancellationTime = time(NULL);
                if (r.driverId != -1)
                {
                    FILE *dfp = fopen(DRIVERS_FILE, "rb+");
                    if (dfp)
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
                    r.driverId = -1;
                }
            }
        }
        fwrite(&r, sizeof(Ride), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    remove(RIDES_FILE);
    rename(TEMP_RIDES_FILE, RIDES_FILE);

    if (!found)
        printf("<p>No ride found with ID %d for this customer.</p>", rideId);
    else
        printf("<p>Ride %d cancelled successfully!</p>", rideId);
}

// ----------------- Rate Driver Form -----------------
void rateDriverForm(int custId, int rideId, int driverId)
{
    if (driverId <= 0)
        return; // no driver assigned

    printf("<h3>Rate Your Driver</h3>");
    printf("<form action='customer_dashboard.cgi' method='GET'>");
    printf("<input type='hidden' name='action' value='submitRating'>");
    printf("<input type='hidden' name='custId' value='%d'>", custId);
    printf("<input type='hidden' name='rideId' value='%d'>", rideId);
    printf("<input type='hidden' name='driverId' value='%d'>", driverId);
    printf("Rating (1-5): <input type='number' name='rating' min='1' max='5' required>");
    printf("<input type='submit' value='Submit Rating'>");
    printf("</form>");
}

// ----------------- Submit Rating -----------------
void submitRating(int custId, int rideId, int driverId, float rating)
{
    if (!hasRated(rideId, custId))
    {
        addRating(rideId, custId, driverId, rating);
        printf("<p>Thank you! Your rating has been submitted.</p>");
    }
    else
    {
        printf("<p>You have already rated this ride.</p>");
    }
    printf("<br><a href='customer_dashboard.cgi?action=viewStatus&custId=%d'>Back to Ride Status</a>", custId);
}

// ----------------- View Ride Status -----------------
void viewRideStatus(int custId)
{
    Ride *rides = NULL;
    int n = loadAllRides(&rides);

    printf("<h3>Ride Status</h3>");
    printf("<table border='1'><tr>"
           "<th>Ride ID</th><th>Pickup</th><th>Drop</th><th>Status</th><th>Driver ID</th>"
           "<th>Booking Time</th><th>Start Time</th><th>End Time</th><th>Cancellation Time</th><th>Rating</th></tr>");

    for (int i = 0; i < n; i++)
    {
        if (rides[i].customerId == custId)
        {
            printf("<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>%d</td><td>",
                   rides[i].rideID, rides[i].pickup, rides[i].drop, rides[i].status, rides[i].driverId);
            printTime(rides[i].bookingTime);
            printf("</td><td>");
            printTime(rides[i].startTime);
            printf("</td><td>");
            printTime(rides[i].endTime);
            printf("</td><td>");
            printTime(rides[i].cancellationTime);
            printf("</td><td>");

            if (strcmp(rides[i].status, "Completed") == 0 && rides[i].driverId != -1)
            {
                if (!hasRated(rides[i].rideID, custId))
                    rateDriverForm(custId, rides[i].rideID, rides[i].driverId);
                else
                    printf("Rated");
            }
            else
                printf("--");

            printf("</td></tr>");
        }
    }

    printf("</table>");
    free(rides);
    printf("<br><a href='customer_dashboard.html?action=dashboard&custId=%d'>Return to Dashboard</a>", custId);
}

// ----------------- View History -----------------
void viewHistory(int custId)
{
    Ride *rides = NULL;
    int n = loadAllRides(&rides);

    printf("<h3>Ride History</h3>");
    printf("<table class='rides-table'><tr>"
           "<th>Ride ID</th><th>Pickup</th><th>Drop</th><th>Status</th>"
           "<th>Booking Time</th><th>Cancel Time</th><th>Rating</th></tr>");

    for (int i = 0; i < n; i++)
    {
        if (rides[i].customerId == custId)
        {
            printf("<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td>",
                   rides[i].rideID, rides[i].pickup, rides[i].drop, rides[i].status);
            printTime(rides[i].bookingTime);
            printf("</td><td>");
            printTime(rides[i].cancellationTime);
            printf("</td><td>");

            if (strcmp(rides[i].status, "Completed") == 0 && rides[i].driverId != -1)
            {
                if (!hasRated(rides[i].rideID, custId))
                    rateDriverForm(custId, rides[i].rideID, rides[i].driverId);
                else
                    printf("Rated");
            }
            else
                printf("N/A");

            // ---------- Bill Section ----------
            printf("</td><td>");
            if (strcmp(rides[i].status, "Completed") == 0)
            {
                // small form to trigger generateBill inside same CGI
                printf("<form method='post' action='customer_dashboard.cgi' style='display:inline;'>");
                printf("<input type='hidden' name='action' value='generateBill'>");
                printf("<input type='hidden' name='rideId' value='%d'>", rides[i].rideID);
                printf("<input type='hidden' name='custId' value='%d'>", custId);
                printf("<input type='submit' value='Download Bill'>");
                printf("</form>");
            }
            else
                printf("N/A");

            printf("</td></tr>");
        }
    }

    printf("</table>");
    free(rides);
    printf("<br><a href='customer_dashboard.html?action=dashboard&custId=%d'>Return to Dashboard</a>", custId);
}
// ----------------- Update Profile -----------------
void updateProfileForm(int custId)
{
    Customer *customers = NULL;
    int n = loadAllCustomers(&customers);
    Customer current = {0};
    for (int i = 0; i < n; i++)
        if (customers[i].id == custId)
            current = customers[i];

    printf("<h3>Update Profile</h3>");
    printf("<form action='customer_dashboard.cgi' method='GET'>");
    printf("<input type='hidden' name='action' value='updateProfile'>");
    printf("<input type='hidden' name='custId' value='%d'>", custId);
    printf("Name: <input type='text' name='name' value='%s' required><br><br>", current.name);
    printf("Phone: <input type='text' name='phone' value='%s' required><br><br>", current.phone);
    printf("<input type='submit' value='Update'>");
    printf("</form>");

    free(customers);
}

void updateProfile(int custId, const char *name, const char *phone)
{
    Customer *customers = NULL;
    int n = loadAllCustomers(&customers);
    int updated = 0;

    for (int i = 0; i < n; i++)
    {
        if (customers[i].id == custId)
        {
            strcpy(customers[i].name, name);
            strcpy(customers[i].phone, phone);
            updated = 1;
            break;
        }
    }

    if (updated && writeAllCustomers(customers, n))
        printf("<p>Profile updated successfully!</p>");
    else
        printf("<p>Error updating profile!</p>");

    free(customers);
    printf("<br><a href='customer_dashboard.html?action=dashboard&custId=%d'>Return to Dashboard</a>", custId);
}

// ----------------- Change Password Form -----------------
void changePasswordForm(int custId)
{
    printf("<h3>Change Password</h3>");
    printf("<form action='customer_dashboard.cgi' method='GET'>");
    printf("<input type='hidden' name='action' value='changePassword'>");
    printf("<input type='hidden' name='custId' value='%d'>", custId);

    printf("Old Password: <input type='password' name='oldpass' required><br><br>");
    printf("New Password: <input type='password' name='newpass' required><br><br>");
    printf("Confirm New Password: <input type='password' name='confirmpass' required><br><br>");

    printf("<input type='submit' value='Update Password'>");
    printf("</form>");
}

// ----------------- Change Password Logic -----------------
void changePassword(int custId, const char *oldpass, const char *newpass, const char *confirmpass)
{
    Customer *customers = NULL;
    int n = loadAllCustomers(&customers);
    int updated = 0;

    for (int i = 0; i < n; i++)
    {
        if (customers[i].id == custId)
        {
            
            if (strcmp(customers[i].password, oldpass) != 0)
            {
                printf("<p style='color:red;'>Incorrect old password!</p>");
                break;
            }

            if (strcmp(newpass, confirmpass) != 0)
            {
                printf("<p style='color:red;'>New passwords do not match!</p>");
                break;
            }

            strcpy(customers[i].password, newpass);
            updated = 1;
            break;
        }
    }

    if (updated && writeAllCustomers(customers, n))
        printf("<p style='color:green;'>Password updated successfully!</p>");
    else if (!updated)
        printf("<p style='color:red;'>Password update failed!</p>");

    free(customers);
    printf("<br><a href='customer_dashboard.html?action=dashboard&custId=%d'>Return to Dashboard</a>", custId);
}

// ----------------- Main -----------------
int main(void)
{
    char *query = getenv("QUERY_STRING");
    char action[128] = "", custIdStr[32] = "", rideIdStr[32] = "", pickup[128] = "", drop[128] = "", driverIdStr[16] = "", ratingStr[8] = "";
    char name[64] = {0}, phone[32] = {0};
    char oldpass[50] = "", newpass[50] = "", confirmpass[50] = "";

    printHeader("Customer Dashboard | Cabify-C");
    if (query)
    {
        getParam(query, "action", action);
        getParam(query, "custId", custIdStr);
        getParam(query, "rideId", rideIdStr);
        getParam(query, "driverId", driverIdStr);
        getParam(query, "rating", ratingStr);
        getParam(query, "pickup", pickup);
        getParam(query, "drop", drop);
        getParam(query, "oldpass", oldpass);
        getParam(query, "newpass", newpass);
        getParam(query, "confirmpass", confirmpass);
    }

    int custId = atoi(custIdStr);
    int rideId = atoi(rideIdStr);
    int driverId = atoi(driverIdStr);
    float rating = (strlen(ratingStr) > 0) ? atof(ratingStr) : 0.0;

    if (custId <= 0)
    {
        printf("<p>Invalid Customer ID</p>");
        printFooter();
        return 0;
    }
    city = initDehradunMap();
    Customer *customers = NULL;
    int n = loadAllCustomers(&customers);
    Customer current = {0};
    for (int i = 0; i < n; i++)
        if (customers[i].id == custId)
            current = customers[i];
    free(customers);

    if (strcmp(action, "dashboard") == 0 || action[0] == '\0')
        showCustomerDashboard(current);
    else if (strcmp(action, "bookRideForm") == 0)
        bookRideForm(custId);
    else if (strcmp(action, "bookRide") == 0)
        bookRide(city, custId, pickup, drop);
    else if (strcmp(action, "cancelRideForm") == 0)
        cancelRideForm(custId);
    else if (strcmp(action, "cancelRide") == 0)
        cancelRide(custId, rideId);
    else if (strcmp(action, "viewHistory") == 0)
        viewHistory(custId);
    else if (strcmp(action, "viewStatus") == 0)
        viewRideStatus(custId);
    else if (strcmp(action, "updateProfileForm") == 0)
        updateProfileForm(custId);
    else if (strcmp(action, "updateProfile") == 0)
        updateProfile(custId, getParam(query, "name", name), getParam(query, "phone", phone));
    else if (strcmp(action, "submitRating") == 0)
        submitRating(custId, rideId, driverId, rating);
    else if (strcmp(action, "changePasswordForm") == 0)
        changePasswordForm(custId);
    else if (strcmp(action, "changePassword") == 0)
        changePassword(custId, oldpass, newpass, confirmpass);
    else if (strcmp(action, "generateBill") == 0)
    {
        char rideIdStr[20], custIdStr[20];
        getParam(query, "rideId", rideIdStr);
        getParam(query, "custId", custIdStr);

        int rideId = atoi(rideIdStr);
        int custId = atoi(custIdStr);

        RideStack s;
        initStack(&s); // only if you use RideStack elsewhere

        generateBill(&s, rideId, custId, 0, "", "", 0, 0.0);
    }
    else
        printf("<p>Invalid action!</p>");

    printFooter();
    return 0;
}
