#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "admin.h"
#include "driver.h"
#include "ride.h"
#include "billing.h"
#include "customer.h"
#include "ratings.h"


void printHeader()
{
    printf("Content-type: text/html\n\n");
    printf("<html><head><title>Admin Dashboard</title>");
    printf("<link rel='stylesheet' href='/CAB_BOOKING/frontend/dashboard.css'>");
    printf("</head><body>");
}

void printFooter()
{
    printf("</body></html>");
}

/* ----------------------- URL Decoding Helper -------------------- */
void urlDecode(char *src, char *dest)
{
    char a, b;
    while (*src)
    {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b)))
        {
            a = (a <= '9' ? a - '0' : toupper(a) - 'A' + 10);
            b = (b <= '9' ? b - '0' : toupper(b) - 'A' + 10);
            *dest++ = 16 * a + b;
            src += 3;
        }
        else if (*src == '+')
        {
            *dest++ = ' ';
            src++;
        }
        else
        {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

/* -------------------------- Get Parameter Utility -------------------------- */
char *getParam(char *query, const char *key, char *value)
{
    char *pos = strstr(query, key);
    if (pos)
    {
        pos = strchr(pos, '=');
        if (pos)
        {
            pos++;
            sscanf(pos, "%[^&]", value);
            return value;
        }
    }
    strcpy(value, "");
    return value;
}


int main(void)
{
    char *query = getenv("QUERY_STRING");
    if (!query)
        query = "";

    char action[64] = "", idStr[32] = "", name[100] = "", phone[100] = "", vehicle[100] = "";
    char rideIdStr[16] = "";
    getParam(query, "action", action);
    getParam(query, "id", idStr);
    getParam(query, "rideId", rideIdStr);
    getParam(query, "name", name);
    getParam(query, "phone", phone);
    getParam(query, "vehicle", vehicle);

    // Decode URL-encoded inputs
    char decodedName[100], decodedPhone[100], decodedVehicle[100];
    urlDecode(name, decodedName);
    urlDecode(phone, decodedPhone);
    urlDecode(vehicle, decodedVehicle);
    strcpy(name, decodedName);
    strcpy(phone, decodedPhone);
    strcpy(vehicle, decodedVehicle);
    if (strcmp(action, "downloadBill") == 0)
    {
        downloadBill(atoi(rideIdStr));
        return 0;
    }
    else if (strcmp(action, "downloadReport") == 0)
    {
        downloadReport();
        return 0;
    }
    printHeader();
    printf("<main class='content'>");
    // ---------------------- Action Handling ----------------------
    if (strcmp(action, "add_driver") == 0)
        manage_driver("add_driver", idStr, name, phone, vehicle);

    else if (strcmp(action, "modify_driver") == 0)
        manage_driver("modify_driver", idStr, name, phone, vehicle);

    else if (strcmp(action, "delete_driver") == 0)
        manage_driver("delete_driver", idStr, name, phone, vehicle);

    else if (strcmp(action, "view_drivers") == 0)
        manage_driver("view_driver", NULL, NULL, NULL, NULL);

    else if (strcmp(action, "view_rides") == 0)
        viewAllRides();

    else if (strcmp(action, "view_bills") == 0)
        displayAllBills();

    else if (strcmp(action, "view_customers") == 0)
        manage_customer("view_all", idStr);

    else if (strcmp(action, "remove_customer") == 0)
        manage_customer("remove", idStr);

    else if (strcmp(action, "block_unblock_customer") == 0)
        manage_customer("toggle_block", idStr);

    else if (strcmp(action, "view_ratings") == 0)
        viewDriverRatings();
    else
    {
        printf("<div style='text-align:center; margin-top:100px;'>");
        printf("<h2>Redirecting to Dashboard...</h2>");
        printf("<p>Please wait a moment.</p>");
        printf("<meta http-equiv='refresh' content='2;url=/CAB_BOOKING/frontend/admin_dashboard.html'>");
        printf("</div>");
    }

    printf("</main>");
    printFooter();

    return 0;
}

/* ----------------------------- Ride Display ----------------------------- */
void viewAllRides()
{
    printf("<div id='view_rides' class='page'>");

    FILE *fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt", "rb");
    if (!fp)
    {
        printf("<p>No rides found!</p>");
        printf("</div>");
        return;
    }

    Ride r;
    int found = 0;
    showPerformanceInsights();
    printf("<div style='margin-bottom:15px;'>");
    printf("<a href='/cgi-bin/admin_dashboard.cgi?action=downloadReport' "
           "style='padding:10px 20px; background:#4CAF50; color:white; text-decoration:none; border-radius:5px;'>Download Report</a>");
    printf("</div>");

    printf("<h2>All Rides</h2>");
    printf("<table border='1' cellpadding='5' cellspacing='0'>");
    printf("<tr>"
           "<th>Ride ID</th><th>Customer ID</th><th>Driver ID</th><th>Pickup</th><th>Drop</th>"
           "<th>Fare</th><th>Status</th><th>Download Bill</th>"
           "</tr>");

    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        printf("<tr>");
        printf("<td>%d</td><td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%.2f</td><td>%s</td>",
               r.rideID, r.customerId, r.driverId, r.pickup, r.drop, r.fare, r.status);
        printf("<td><a href='/cgi-bin/admin_dashboard.cgi?action=downloadBill&rideId=%d'>Download </a></td>", r.rideID);
        printf("</tr>");
        found = 1;
    }

    printf("</table>");
    if (!found)
        printf("<p>No rides recorded yet.</p>");

    fclose(fp);
    printf("</div>");
}

void showPerformanceInsights()
{
    FILE *ridesFile = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt", "rb");
    if (!ridesFile)
    {
        printf("<p>Error loading rides data.</p>");
        return;
    }

    Ride r;
    int totalRides = 0;
    float totalRevenue = 0;

    while (fread(&r, sizeof(Ride), 1, ridesFile))
    {
        totalRides++;
        totalRevenue += r.fare;
    }

    printf("<div id='performance-insights' style='margin:20px 0; padding:10px; border:1px solid #ccc; border-radius:5px; background:#f9f9f9;'>");
    printf("<h3>Performance Insights📈</h3>");
    printf("<p><strong>Total Rides:</strong> %d | <strong>Total Revenue:</strong> %.2f</p>", totalRides, totalRevenue);
    if (totalRides > 0)
        printf("<p><strong>Average Fare:</strong> %.2f</p>", totalRevenue / totalRides);
    printf("</div>");

    fclose(ridesFile);
}

void formatTime(char *buffer, time_t t)
{
    if (t <= 0)
    {
        strcpy(buffer, "N/A");
        return;
    }

    struct tm *tm_info = localtime(&t);
    if (tm_info)
        strftime(buffer, 25, "%d-%m-%Y %H:%M", tm_info);
    else
        strcpy(buffer, "N/A");
}

/* ------------------- Download Single Ride Bill ------------------- */
void downloadBill(int rideId)
{
    FILE *ridesFile = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt", "rb");
    if (!ridesFile)
    {
        printf("Content-Type:text/html\r\n\r\nError opening rides file.");
        return;
    }

    printf("Content-Type:text/html\r\n\r\n");

    Ride r;
    int found = 0;

    while (fread(&r, sizeof(Ride), 1, ridesFile) == 1)
    {
        if (r.rideID == rideId)
        {
            found = 1;

            printf("<!DOCTYPE html>");
            printf("<html><head><meta charset='UTF-8'>");
            printf("<title>Cabify-C Ride Bill #%d</title>", r.rideID);
            printf("<style>"
                   "body{font-family:'Segoe UI',Arial,sans-serif;background:#f4f6f8;margin:0;padding:0;}"
                   ".bill-container{max-width:700px;margin:50px auto;background:#fff;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.1);padding:30px;}"
                   ".header{text-align:center;border-bottom:2px solid #4CAF50;padding-bottom:10px;}"
                   ".header img{height:60px;margin-bottom:10px;}"
                   ".header h1{margin:5px 0;color:#4CAF50;}"
                   ".bill-info{margin-top:20px;}"
                   ".bill-info table{width:100%%;border-collapse:collapse;margin-top:10px;}"
                   "th,td{border:1px solid #ddd;padding:10px;text-align:left;}"
                   "th{background-color:#4CAF50;color:white;}"
                   ".summary{margin-top:20px;text-align:right;}"
                   ".summary p{margin:5px 0;font-size:16px;}"
                   ".footer{text-align:center;margin-top:30px;color:#777;font-size:13px;}"
                   ".btn-print{display:inline-block;margin-top:20px;background:#4CAF50;color:white;padding:10px 20px;border:none;border-radius:5px;cursor:pointer;font-size:16px;}"
                   ".btn-print:hover{background:#45a049;}"
                   "</style></head><body>");

            printf("<div class='bill-container'>");
            printf("<div class='header'>");
            printf("<h1>🚖Ride Invoice</h1>");
            printf("<p>Cabify-C Online Cab Booking</p>");
            printf("</div>");

            printf("<div class='bill-info'>");
            printf("<h3>Ride Details</h3>");
            printf("<table>");
            printf("<tr><th>Ride ID</th><td>%d</td></tr>", r.rideID);
            printf("<tr><th>Customer ID</th><td>%d</td></tr>", r.customerId);
            printf("<tr><th>Driver ID</th><td>%d</td></tr>", r.driverId);
            printf("<tr><th>Pickup Location</th><td>%s</td></tr>", r.pickup);
            printf("<tr><th>Drop Location</th><td>%s</td></tr>", r.drop);
            printf("<tr><th>Distance</th><td>%d km</td></tr>", r.distance);
            printf("<tr><th>Status</th><td>%s</td></tr>", r.status);
            printf("</table>");

            // Summary Section (fare + tax + total)
            float baseFare = r.fare;
            float gst = baseFare * 0.05;
            float total = baseFare + gst;

            printf("<div class='summary'>");
            printf("<p><strong>Base Fare:</strong> ₹%.2f</p>", baseFare);
            printf("<p><strong>GST (5%%):</strong> ₹%.2f</p>", gst);
            printf("<hr style='border:none;border-top:1px solid #ddd;'>");
            printf("<p style='font-size:18px;'><strong>Total Amount:</strong> ₹%.2f</p>", total);
            printf("</div>");

            printf("<div class='footer'>");
            printf("<button class='btn-print' onclick='window.print()'>🖨️ Print / Save as PDF</button>");
            printf("<p>Thank you for riding with Cabify-C!</p>");
            printf("<p>For support: support@cabify-c.in | +91 99999 99999</p>");
            printf("</div>");

            printf("</div></body></html>");
            break;
        }
    }

    if (!found)
        printf("<h3>Ride not found!</h3>");

    fclose(ridesFile);
}

/* ------------------- Download Full Report ------------------- */
void downloadReport()
{
    FILE *ridesFile = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt", "rb");
    if (!ridesFile)
    {
        printf("Content-Type:text/html\r\n\r\nError opening rides file.");
        return;
    }

    printf("Content-Type:text/html\r\n\r\n");
    printf("<!DOCTYPE html>");
    printf("<html lang='en'><head><meta charset='UTF-8'>");
    printf("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    printf("<title>Cabify-C Ride Report</title>");
    printf("<style>"
           "body{font-family:'Segoe UI',Arial,sans-serif;background:#f5f7fa;margin:0;padding:0;}"
           ".report-container{max-width:90%%;margin:30px auto;background:#fff;border-radius:12px;"
           "box-shadow:0 4px 12px rgba(0,0,0,0.1);padding:30px;}"
           ".header{text-align:center;margin-bottom:20px;}"
           ".header h1{color:#2e7d32;margin:10px 0;font-size:28px;}"
           ".header h2{color:#333;margin:5px 0;font-size:20px;}"
           ".header p{color:#777;font-size:14px;margin:0;}"
           "table{width:100%%;border-collapse:collapse;margin-top:25px;font-size:15px;}"
           "th,td{border:1px solid #ddd;padding:10px;text-align:center;}"
           "th{background:#43a047;color:#fff;font-weight:600;}"
           "tr:nth-child(even){background:#f9f9f9;}"
           "tr:hover{background:#eefbee;}"
           ".summary{margin-top:25px;text-align:right;font-size:16px;}"
           ".summary strong{color:#2e7d32;}"
           ".btn-print{display:block;margin:30px auto 10px auto;background:#43a047;color:white;"
           "padding:12px 25px;border:none;border-radius:6px;cursor:pointer;font-size:16px;}"
           ".btn-print:hover{background:#388e3c;}"
           ".footer{text-align:center;margin-top:20px;color:#777;font-size:14px;}"
           ".footer a{color:#2e7d32;text-decoration:none;}"
           ".footer a:hover{text-decoration:underline;}"
           "</style></head><body>");

    printf("<div class='report-container'>");
    printf("<div class='header'>");
    printf("<h2>🚖 Cabify-C</h2>");
    printf("<h1>Complete Ride Report</h1>");
    printf("<p>Cabify-C Online Cab Booking | Generated on: ");

    time_t now = time(NULL);
    printf("%s</p>", ctime(&now));
    printf("</div>");

    printf("<table>");
    printf("<tr>"
           "<th>Ride ID</th><th>Customer ID</th><th>Driver ID</th>"
           "<th>Pickup</th><th>Drop</th><th>Distance (km)</th>"
           "<th>Fare (₹)</th><th>Status</th>"
           "<th>Booking Time</th><th>Start Time</th><th>End Time</th>"
           "</tr>");

    Ride r;
    float totalRevenue = 0;
    int totalRides = 0;
    char bookingStr[25], startStr[25], endStr[25];

    while (fread(&r, sizeof(Ride), 1, ridesFile) == 1)
    {
        totalRides++;
        totalRevenue += r.fare;

        formatTime(bookingStr, r.bookingTime);
        formatTime(startStr, r.startTime);
        formatTime(endStr, r.endTime);

        printf("<tr>");
        printf("<td>%d</td><td>%d</td><td>%d</td>"
               "<td>%s</td><td>%s</td><td>%d</td><td>₹%.2f</td>"
               "<td>%s</td><td>%s</td><td>%s</td><td>%s</td>",
               r.rideID, r.customerId, r.driverId,
               r.pickup, r.drop, r.distance, r.fare,
               r.status, bookingStr, startStr, endStr);
        printf("</tr>");
    }

    printf("</table>");

    // Summary Section
    printf("<div class='summary'>");
    printf("<hr style='border:none;border-top:1px solid #ddd;margin:15px 0;'>");
    printf("<p><strong>Total Rides:</strong> %d</p>", totalRides);
    printf("<p><strong>Total Revenue:</strong> ₹%.2f</p>", totalRevenue);
    if (totalRides > 0)
        printf("<p><strong>Average Fare:</strong> ₹%.2f</p>", totalRevenue / totalRides);
    printf("</div>");

    // Print Button + Footer
    printf("<button class='btn-print' onclick='window.print()'>🖨️ Print / Save as PDF</button>");
    printf("<div class='footer'>");
    printf("<p>Cabify-C Report System | <a href='mailto:support@cabify-c.in'>support@cabify-c.in</a> | +91 99999 99999</p>");
    printf("</div>");

    printf("</div></body></html>");
    fclose(ridesFile);
}
