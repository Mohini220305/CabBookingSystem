#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "map.h"
#include "driver.h"
#include "ride.h"
#include "stack.h"
#include "billing.h"

#define DRIVER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt"
#define RIDES_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt"
#define CUSTOMER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt"
#define BILLS_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/bills.txt"

extern Graph *city;
extern int getLocationIndex(Graph *g, char placeName[]);                    
extern int calculateDistance(Graph *g, int a, int b);                       
extern void reassignRide(Ride *r, Graph *g, int rejectingDriverId);        
extern void pushBill(Ride *r);                                              
extern void pushStack(RideStack *s, Ride r);                              
extern RideStack rs;                                                        

int updateDriverInFile(int driverId, Driver newData)
{
    FILE *fp = fopen(DRIVER_FILE, "rb+");
    if (!fp)
    {
        printf("<p style='color:red;'>Error: Unable to open driver file!</p>");
        return 0;
    }

    Driver d;
    int found = 0;

    while (fread(&d, sizeof(Driver), 1, fp) == 1)
    {
        if (d.id == driverId)
        {
            found = 1;
            fseek(fp, -sizeof(Driver), SEEK_CUR);
            fwrite(&newData, sizeof(Driver), 1, fp);
            fflush(fp);
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        printf("<p style='color:red;'>Driver with ID %d not found!</p>", driverId);
        return 0;
    }

    return 1;
}

void getParam(const char *query, const char *key, char *out, size_t outsz)
{
    out[0] = '\0';
    if (!query || !key)
        return;

    size_t klen = strlen(key);
    const char *p = query;
    while ((p = strstr(p, key)) != NULL)
    {
        /* ensure it matches key= not substring */
        if (p[klen] == '=')
        {
            p += klen + 1;
            size_t i = 0;
            while (*p && *p != '&' && i + 1 < outsz)
            {
                /* simple decode of + into space and %xx -> char */
                if (*p == '+')
                {
                    out[i++] = ' ';
                    p++;
                    continue;
                }
                if (*p == '%' && isxdigit((unsigned char)p[1]) && isxdigit((unsigned char)p[2]))
                {
                    char hex[3] = {p[1], p[2], 0};
                    out[i++] = (char)strtol(hex, NULL, 16);
                    p += 3;
                    continue;
                }
                out[i++] = *p++;
            }
            out[i] = '\0';
            return;
        }
        else
        {
            p = p + klen;
        }
    }
}

int getDriverById(int id, Driver *d)
{
    FILE *fp = fopen(DRIVER_FILE, "rb");
    if (!fp)
        return 0;
    while (fread(d, sizeof(Driver), 1, fp) == 1)
    {
        if (d->id == id)
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void htmlHeader()
{
    printf("Content-Type: text/html\r\n\r\n");
    printf("<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>");
    printf("<title>Driver Dashboard</title>");
    printf("<style>"
           "body{font-family:Arial,Helvetica,sans-serif;margin:12px;color:#222}"
           ".driver-info{background:#f9f9f9;border-radius:8px;padding:12px;margin-bottom:12px}"
           "table{border-collapse:collapse;width:100%%;margin-top:10px}"
           "th,td{border:1px solid #ddd;padding:8px;text-align:center}"
           "th{background:#f2f2f2}"
           "button, input[type=submit]{padding:6px 10px;border-radius:6px;border:0;background:#007bff;color:#fff;cursor:pointer}"
           ".danger{background:#e74c3c}"
           ".muted{color:#666;font-size:0.95rem}"
           ".small{font-size:0.9rem}"
           "form.inline{display:inline-block;margin:0 6px}"
           "</style></head><body>");
}

void htmlFooter()
{
    printf("</body></html>");
}

void updateDriverAfterRide(int driverId, int dropIndex, float fare)
{
    FILE *fp = fopen(DRIVER_FILE, "rb+");
    if (!fp)
    {
        printf("<p>Error opening drivers file.</p>");
        return;
    }

    Driver d;
    while (fread(&d, sizeof(Driver), 1, fp) == 1)
    {
        if (d.id == driverId)
        {
            if (dropIndex != -1)
                d.location = dropIndex;
            d.available = 1;
            d.completedRides += 1;
            d.earnings += fare;

            fseek(fp, -sizeof(Driver), SEEK_CUR);
            fwrite(&d, sizeof(Driver), 1, fp);
            fclose(fp);
            return;
        }
    }

    fclose(fp);
    printf("<p class='muted'>Driver not found while updating ride.</p>");
}

void updateCustomerAfterRide(int customerId)
{
    FILE *fp = fopen(CUSTOMER_FILE, "rb+");
    if (!fp)
    {
        printf("<p>Error opening customer file.</p>");
        return;
    }

    Customer c;
    while (fread(&c, sizeof(Customer), 1, fp) == 1)
    {
        if (c.id == customerId)
        {
            c.totalRides += 1;
            fseek(fp, -sizeof(Customer), SEEK_CUR);
            fwrite(&c, sizeof(Customer), 1, fp);
            fclose(fp);
            return;
        }
    }

    fclose(fp);
    printf("<p class='muted'>Customer not found while updating total rides.</p>");
}

void showDriverDashboardHtml(Driver d)
{
    printf("<h2>Welcome, %s</h2>", d.name);
       printf("<div class='driver-info'>");
    printf("<p><b>Driver ID:</b> %d &nbsp; | &nbsp; <b>Vehicle:</b> %s &nbsp; | &nbsp; <b>Phone:</b> %s</p>",
           d.id, d.vehicle, d.phone);
    const char *loc = "Unknown";
    if (city && d.location >= 0 && d.location < city->n)
        loc = city->placeNames[d.location];
    printf("<p><b>Location:</b> %s &nbsp; | &nbsp; <b>Status:</b> %s</p>",
           loc, d.available ? "Online" : "Offline");
    printf("<p><b>Completed Rides:</b> %d &nbsp; | &nbsp; <b>Earnings:</b> ₹%.2f</p>",
           d.completedRides, d.earnings);
    /* Availability toggle form */
    printf("<form class='inline' method='GET'><input type='hidden' name='action' value='toggleAvailability'>"
           "<input type='hidden' name='driverId' value='%d'>"
           "<input type='hidden' name='set' value='%d'>"
           "<input type='submit' value='%s'></form>",
           d.id, d.available ? 0 : 1, d.available ? "Go Offline" : "Go Online");
    printf("</div>");
}

void viewAssignedRidesHtml(int driverId)
{
    FILE *fp = fopen(RIDES_FILE, "rb");
    if (!fp)
    {
        printf("<p>No rides found.</p>");
        return;
    }

    Ride r;
    int found = 0;
    printf("<h3>Assigned Rides</h3>");
    printf("<table class='rides-table'><tr><th>Ride ID</th><th>Customer ID</th><th>Pickup</th><th>Drop</th><th>Status</th><th>Booking Time</th><th>Actions</th></tr>");
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.driverId == driverId &&
            (strcmp(r.status, "Waiting") == 0 ||
             strcmp(r.status, "Pending") == 0 ||
             strcmp(r.status, "Accepted") == 0 ||
             strcmp(r.status, "Ongoing") == 0))
        {

            found = 1;
            char bookingBuf[64] = {0};
            if (r.bookingTime > 0)
            {
                struct tm *bt = localtime(&r.bookingTime);
                strftime(bookingBuf, sizeof(bookingBuf), "%Y-%m-%d %H:%M:%S", bt);
            }

            printf("<tr>");
            printf("<td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%s</td><td class='small'>%s</td>",
                   r.rideID, r.customerId, r.pickup, r.drop, r.status, bookingBuf[0] ? bookingBuf : "N/A");

            /* Actions column: present forms depending on status */
            printf("<td>");
            if (strcmp(r.status, "Pending") == 0 || strcmp(r.status, "Waiting") == 0)
            {
                /* Accept form */
                printf("<form class='inline' method='GET'>"
                       "<input type='hidden' name='action' value='acceptRide'>"
                       "<input type='hidden' name='driverId' value='%d'>"
                       "<input type='hidden' name='rideId' value='%d'>"
                       "<input type='submit' value='Accept'></form>",
                       driverId, r.rideID);

                /* Reject form */
                printf("<form class='inline' method='GET'>"
                       "<input type='hidden' name='action' value='rejectRide'>"
                       "<input type='hidden' name='driverId' value='%d'>"
                       "<input type='hidden' name='rideId' value='%d'>"
                       "<input type='submit' value='Reject' class='danger'></form>",
                       driverId, r.rideID);
            }
            else if (strcmp(r.status, "Accepted") == 0)
            {
                /* Start ride */
                printf("<form class='inline' method='GET'>"
                       "<input type='hidden' name='action' value='startRide'>"
                       "<input type='hidden' name='driverId' value='%d'>"
                       "<input type='hidden' name='rideId' value='%d'>"
                       "<input type='submit' value='Start'></form>",
                       driverId, r.rideID);
            }
            else if (strcmp(r.status, "Ongoing") == 0)
            {
                /* End ride (complete) */
                printf("<form class='inline' method='GET'>"
                       "<input type='hidden' name='action' value='endRide'>"
                       "<input type='hidden' name='driverId' value='%d'>"
                       "<input type='hidden' name='rideId' value='%d'>"
                       "<input type='submit' value='End' class='danger'></form>",
                       driverId, r.rideID);
            }
            else
            {
                printf("-");
            }
            printf("</td>");

            printf("</tr>");
        }
    }
    printf("</table>");
    if (!found)
        printf("<p class='muted'>No assigned rides available.</p>");
    fclose(fp);
}

void viewCompletedRidesHtml(int driverId)
{
    FILE *fp = fopen(RIDES_FILE, "rb");
    if (!fp)
    {
        printf("<p>No rides found.</p>");
        return;
    }

    Ride r;
    int found = 0;
    printf("<h3>Completed Rides</h3>");
    printf("<table class='rides-table'><tr><th>Ride ID</th><th>Pickup</th><th>Drop</th><th>Distance</th><th>Fare</th><th>End Time</th></tr>");
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.driverId == driverId && strcmp(r.status, "Completed") == 0)
        {
            found = 1;
            char endBuf[64] = {0};
            if (r.endTime > 0)
            {
                struct tm *et = localtime(&r.endTime);
                strftime(endBuf, sizeof(endBuf), "%Y-%m-%d %H:%M:%S", et);
            }
            printf("<tr><td>%d</td><td>%s</td><td>%s</td><td>%d km</td><td>₹%.2f</td><td class='small'>%s</td></tr>",
                   r.rideID, r.pickup, r.drop, r.distance, r.fare, endBuf[0] ? endBuf : "N/A");
        }
    }
    printf("</table>");
    if (!found)
        printf("<p class='muted'>No completed rides.</p>");
    fclose(fp);
}

void updateRideRecord(Ride *r)
{
    FILE *fp = fopen(RIDES_FILE, "rb+");
    if (!fp)
        return;

    Ride temp;
    while (fread(&temp, sizeof(Ride), 1, fp))
    {
        if (temp.rideID == r->rideID)
        {
            fseek(fp, -sizeof(Ride), SEEK_CUR);
            fwrite(r, sizeof(Ride), 1, fp);
            break;
        }
    }
    fclose(fp);
}

/* Accept ride */
void acceptRideHandler(int driverId, int rideId)
{
    FILE *fp = fopen(RIDES_FILE, "rb+");
    if (!fp)
    {
        printf("<p>Error opening rides file.</p>");
        return;
    }

    Ride r;
    int found = 0;
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.rideID == rideId &&
            (strcmp(r.status, "Pending") == 0 || strcmp(r.status, "Waiting") == 0))
        {
            found = 1;
            strcpy(r.status, "Accepted");
            r.driverId = driverId;
            r.startTime = 0;
            r.endTime = 0;
            updateRideRecord(&r);
            printf("<p>✅ Ride %d accepted successfully.</p>", rideId);
            break;
        }
    }

    fclose(fp);

    if (!found)
        printf("<p class='muted'>Ride not found or not pending.</p>");

    /* 🔁 Reload the updated data fresh */
    viewAssignedRidesHtml(driverId);
}

/* Reject ride: set driver available again and reassign */
void rejectRideHandler(int driverId, int rideId)
{
    FILE *dfp = fopen(DRIVER_FILE, "rb+");
    if (dfp)
    {
        Driver d;
        while (fread(&d, sizeof(Driver), 1, dfp) == 1)
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

    FILE *fp = fopen(RIDES_FILE, "rb");
    if (!fp)
    {
        printf("<p>Error opening rides file.</p>");
        return;
    }

    Ride r;
    int found = 0;
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.rideID == rideId &&
            (strcmp(r.status, "Pending") == 0 || strcmp(r.status, "Waiting") == 0))
        {
            found = 1;
            enqueueRide(&r);
            reassignRide(&r, city, driverId);
            printf("<p>❌ Ride %d rejected and reassigned.</p>", rideId);
            break;
        }
    }

    fclose(fp);

    if (!found)
        printf("<p class='muted'>Ride not found or not pending.</p>");

    /* Reload updated data */
    viewAssignedRidesHtml(driverId);
}

/* Start ride */
void startRideHandler(int driverId, int rideId)
{
    FILE *fp = fopen(RIDES_FILE, "rb+");
    if (!fp)
    {
        printf("<p>Error opening rides file.</p>");
        return;
    }

    Ride r;
    int found = 0;
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.rideID == rideId && strcmp(r.status, "Accepted") == 0)
        {
            found = 1;
            strcpy(r.status, "Ongoing");
            r.startTime = time(NULL);
            updateRideRecord(&r);
            printf("<p>🚗 Ride %d started.</p>", rideId);
            break;
        }
    }

    fclose(fp);

    if (!found)
        printf("<p class='muted'>Ride not found or not in Accepted state.</p>");

    viewAssignedRidesHtml(driverId);
}

/* End ride (complete) */
void endRideHandler(int driverId, int rideId)
{
    FILE *fp = fopen(RIDES_FILE, "rb+");
    if (!fp)
    {
        printf("<p>Error opening rides file.</p>");
        return;
    }

    Ride r;
    int found = 0;
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        if (r.rideID == rideId && strcmp(r.status, "Ongoing") == 0)
        {
            found = 1;
            strcpy(r.status, "Completed");
            r.endTime = time(NULL);

            int pickupIndex = getLocationIndex(city, r.pickup);
            int dropIndex = getLocationIndex(city, r.drop);
            if (pickupIndex != -1 && dropIndex != -1)
            {
                r.distance = calculateDistance(city, pickupIndex, dropIndex);
                r.fare = calculateFare(city, r.pickup, r.drop);
            }

            updateRideRecord(&r);

            updateDriverAfterRide(driverId, dropIndex, r.fare);
            updateCustomerAfterRide(r.customerId);
            pushBill(&r);

            printf("<p>✅ Ride %d completed. Fare ₹%.2f | %d km</p>", rideId, r.fare, r.distance);
            break;
        }
    }

    fclose(fp);

    if (!found)
        printf("<p class='muted'>Ride not found or not ongoing.</p>");

    viewAssignedRidesHtml(driverId);
}

void showUpdateProfileFormHtml(Driver d)
{
    printf("<h3>Update Profile</h3>");
    printf("<form method='GET'>"
           "<input type='hidden' name='action' value='updateProfileSubmit'>"
           "<input type='hidden' name='driverId' value='%d'>"
           "Name: <input type='text' name='name' value='%s'><br><br>"
           "Phone: <input type='text' name='phone' value='%s'><br><br>"
           "Vehicle: <input type='text' name='vehicle' value='%s'><br><br>"
           "<input type='submit' value='Save'>"
           "</form>",
           d.id, d.name, d.phone, d.vehicle);
}

void handleUpdateProfileSubmit(int driverId, const char *name, const char *phone, const char *vehicle)
{
    Driver d;
    if (!getDriverById(driverId, &d))
    {
        printf("<p>Driver not found.</p>");
        return;
    }
    strncpy(d.name, name, sizeof(d.name) - 1);
    d.name[sizeof(d.name) - 1] = 0;
    strncpy(d.phone, phone, sizeof(d.phone) - 1);
    d.phone[sizeof(d.phone) - 1] = 0;
    strncpy(d.vehicle, vehicle, sizeof(d.vehicle) - 1);
    d.vehicle[sizeof(d.vehicle) - 1] = 0;

    /* try to use updateDriverInFile if available */
    if (updateDriverInFile(driverId, d))
    {
        printf("<p>✅ Profile updated successfully.</p>");
    }
    else
    {
        /* fallback: rewrite entire drivers file */
        Driver drivers[500];
        int nd = loadDrivers(drivers);
        for (int i = 0; i < nd; i++)
        {
            if (drivers[i].id == driverId)
            {
                drivers[i] = d;
                break;
            }
        }
        FILE *dfp = fopen(DRIVER_FILE, "wb");
        if (dfp)
        {
            fwrite(drivers, sizeof(Driver), nd, dfp);
            fclose(dfp);
            printf("<p>✅ Profile updated (fallback).</p>");
        }
        else
            printf("<p class='muted'>Failed to write driver file.</p>");
    }
    showDriverDashboardHtml(d);
}

void showSetLocationFormHtml(Driver d)
{
    printf("<h3>Set Current Location</h3>");
    printf("<form method='GET'>"
           "<input type='hidden' name='action' value='updateLocationSubmit'>"
           "<input type='hidden' name='driverId' value='%d'>"
           "<label>Select Location:</label><br>"
           "<select name='place'>",
           d.id);
    if (city)
    {
        for (int i = 0; i < city->n; i++)
        {
            printf("<option value='%s' %s>%s</option>", city->placeNames[i], (i == d.location) ? "selected" : "", city->placeNames[i]);
        }
    }
    else
    {
        printf("<option value=''>No locations loaded</option>");
    }
    printf("</select><br><br><input type='submit' value='Update Location'></form>");
    printf("<a href='/cgi-bin/display_map.cgi' target='_blank' style='color:#3498db;text-decoration:none;'>");
    printf("🗺️ View City Map</a>");
}

void handleUpdateLocationSubmit(int driverId, const char *place)
{
    Driver d;
    if (!getDriverById(driverId, &d))
    {
        printf("<p>Driver not found.</p>");
        return;
    }
    int idx = getLocationIndex(city, (char *)place);
    if (idx == -1)
    {
        printf("<p class='muted'>Invalid location selected.</p>");
        showSetLocationFormHtml(d);
        return;
    }
    d.location = idx;
    if (updateDriverInFile(driverId, d))
    {
        printf("<p>📍 Location updated to <b>%s</b></p>", place);
    }
    else
    {
        /* fallback rewrite */
        Driver drivers[500];
        int nd = loadDrivers(drivers);
        for (int i = 0; i < nd; i++)
            if (drivers[i].id == driverId)
            {
                drivers[i].location = idx;
                break;
            }
        FILE *dfp = fopen(DRIVER_FILE, "wb");
        if (dfp)
        {
            fwrite(drivers, sizeof(Driver), nd, dfp);
            fclose(dfp);
            printf("<p>📍 Location updated (fallback) to <b>%s</b></p>", place);
        }
        else
            printf("<p class='muted'>Failed to update location.</p>");
    }
    showDriverDashboardHtml(d);
}

void showChangePasswordFormHtml(int driverId)
{
    printf("<h3>Change Password</h3>");
    printf("<form method='GET'>"
           "<input type='hidden' name='action' value='changePasswordSubmit'>"
           "<input type='hidden' name='driverId' value='%d'>"
           "Old Password: <input type='password' name='old'><br><br>"
           "New Password: <input type='password' name='newp'><br><br>"
           "<input type='submit' value='Change Password'>"
           "</form>",
           driverId);
}

void handleChangePasswordSubmit(int driverId, const char *oldp, const char *newp)
{
    Driver d;
    if (!getDriverById(driverId, &d))
    {
        printf("<p>Driver not found.</p>");
        return;
    }
    if (strcmp(d.pass, oldp) != 0)
    {
        printf("<p class='muted'>❌ Incorrect current password.</p>");
        showChangePasswordFormHtml(driverId);
        return;
    }
    strncpy(d.pass, newp, sizeof(d.pass) - 1);
    d.pass[sizeof(d.pass) - 1] = 0;
    d.mustChangePassword = 0;
    if (updateDriverInFile(driverId, d))
    {
        printf("<p>✅ Password changed successfully.</p>");
    }
    else
    {
        Driver drivers[500];
        int nd = loadDrivers(drivers);
        for (int i = 0; i < nd; i++)
            if (drivers[i].id == driverId)
            {
                drivers[i] = d;
                break;
            }
        FILE *dfp = fopen(DRIVER_FILE, "wb");
        if (dfp)
        {
            fwrite(drivers, sizeof(Driver), nd, dfp);
            fclose(dfp);
            printf("<p>✅ Password changed (fallback).</p>");
        }
        else
            printf("<p class='muted'>Failed to update password.</p>");
    }
    showDriverDashboardHtml(d);
}

/* Toggle availability handler */
void toggleAvailabilityHandler(int driverId, int setTo)
{
    Driver d;
    if (!getDriverById(driverId, &d))
    {
        printf("<p>Driver not found.</p>");
        return;
    }
    d.available = setTo ? 1 : 0;
    if (updateDriverInFile(driverId, d))
    {
        printf("<p>Availability updated.</p>");
    }
    else
    {
        Driver drivers[500];
        int nd = loadDrivers(drivers);
        for (int i = 0; i < nd; i++)
            if (drivers[i].id == driverId)
            {
                drivers[i].available = setTo;
                break;
            }
        FILE *dfp = fopen(DRIVER_FILE, "wb");
        if (dfp)
        {
            fwrite(drivers, sizeof(Driver), nd, dfp);
            fclose(dfp);
            printf("<p>Availability updated (fallback).</p>");
        }
        else
            printf("<p class='muted'>Failed to update availability.</p>");
    }
    showDriverDashboardHtml(d);
}

/* ------------------ MAIN CGI ROUTER ------------------ */
int main(void)
{
    char *qs = getenv("QUERY_STRING");
    char action[128] = {0}, driverIdStr[32] = {0}, rideIdStr[32] = {0}, name[128] = {0}, phone[64] = {0}, vehicle[64] = {0}, place[128] = {0}, oldp[64] = {0}, newp[64] = {0}, setStr[8] = {0};

    if (!qs)
        qs = "";

    getParam(qs, "action", action, sizeof(action));
    getParam(qs, "driverId", driverIdStr, sizeof(driverIdStr));
    getParam(qs, "rideId", rideIdStr, sizeof(rideIdStr));
    getParam(qs, "name", name, sizeof(name));
    getParam(qs, "phone", phone, sizeof(phone));
    getParam(qs, "vehicle", vehicle, sizeof(vehicle));
    getParam(qs, "place", place, sizeof(place));
    getParam(qs, "old", oldp, sizeof(oldp));
    getParam(qs, "newp", newp, sizeof(newp));
    getParam(qs, "set", setStr, sizeof(setStr));

    int driverId = atoi(driverIdStr);
    int rideId = atoi(rideIdStr);
    int setVal = atoi(setStr);

    htmlHeader();

    if (driverId <= 0)
    {
        printf("<p class='muted'>Invalid or missing driverId. Please pass driverId in query string.</p>");
        htmlFooter();
        return 0;
    }
    city = initDehradunMap();
    /* try to load driver */
    Driver d;
    if (!getDriverById(driverId, &d))
    {
        printf("<p class='muted'>Driver not found (id=%d)</p>", driverId);
        htmlFooter();
        return 0;
    }

    /* Route actions */
    if (strcmp(action, "") == 0 || strcmp(action, "dashboard") == 0)
    {
        showDriverDashboardHtml(d);
    }
    else if (strcmp(action, "viewAssigned") == 0)
    {
        showDriverDashboardHtml(d);
        viewAssignedRidesHtml(driverId);
    }
    else if (strcmp(action, "viewCompleted") == 0)
    {
        showDriverDashboardHtml(d);
        viewCompletedRidesHtml(driverId);
    }
    else if (strcmp(action, "acceptRide") == 0)
    {
        acceptRideHandler(driverId, rideId);
    }
    else if (strcmp(action, "rejectRide") == 0)
    {
        rejectRideHandler(driverId, rideId);
    }
    else if (strcmp(action, "startRide") == 0)
    {
        startRideHandler(driverId, rideId);
    }
    else if (strcmp(action, "endRide") == 0)
    {
        endRideHandler(driverId, rideId);
    }
    else if (strcmp(action, "updateProfile") == 0)
    {
        showUpdateProfileFormHtml(d);
    }
    else if (strcmp(action, "updateProfileSubmit") == 0)
    {
        handleUpdateProfileSubmit(driverId, name, phone, vehicle);
    }
    else if (strcmp(action, "setLocation") == 0)
    {
        showSetLocationFormHtml(d);
    }
    else if (strcmp(action, "updateLocationSubmit") == 0)
    {
        handleUpdateLocationSubmit(driverId, place);
    }
    else if (strcmp(action, "changePassword") == 0)
    {
        showChangePasswordFormHtml(driverId);
    }
    else if (strcmp(action, "changePasswordSubmit") == 0)
    {
        handleChangePasswordSubmit(driverId, oldp, newp);
    }
    else if (strcmp(action, "toggleAvailability") == 0)
    {
        toggleAvailabilityHandler(driverId, setVal);
    }
    else
    {
        printf("<p class='muted'>Unknown action: %s</p>", action);
    }

    htmlFooter();
    return 0;
}
