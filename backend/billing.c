#include "billing.h"
#include "ride.h"
#include "priorityQueue.h"
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"
#include "admin.h"

RideStack rs;
BillStack billStack = {.top = -1};

void initBillStack()
{
    billStack.top = -1;
}

int isBillStackEmpty()
{
    return billStack.top == -1;
}

int isBillStackFull()
{
    return billStack.top == MAX_BILLS - 1;
}

void pushBill(Ride *ride)
{
    if (isBillStackFull())
    {
        printf("Bill Stack is full! Cannot push new bill.\n");
        return;
    }
    billStack.bills[++billStack.top] = ride;
    //printf("Bill pushed into stack and saved for Admin.\n");
}

Ride *popBill()
{
    if (isBillStackEmpty())
    {
        printf("No bills to display!\n");
        return NULL;
    }
    return billStack.bills[billStack.top--];
}

void loadBillsFromFile()
{
    FILE *fp = fopen("bills.txt", "rb");
    if (!fp)
        return;

    initBillStack(); // clear stack first

    Ride r;
    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        Ride *billCopy = (Ride *)malloc(sizeof(Ride));
        *billCopy = r;
        pushBill(billCopy);
    }

    fclose(fp);
}

float calculateFare(Graph *city, char pickup[], char drop[])
{
    int pickupIndex = getLocationIndex(city, pickup);
    int dropIndex = getLocationIndex(city, drop);

    if (pickupIndex == -1 || dropIndex == -1)
    {
        printf("Invalid pickup/drop location for fare calculation!\n");
        return 0;
    }

    int dist[MAX], visited[MAX];
    for (int i = 0; i < city->n; i++)
    {
        dist[i] = INT_MAX;
        visited[i] = 0;
    }

    PriorityQueue pq;
    initPQ(&pq);
    push(&pq, pickupIndex, 0);
    dist[pickupIndex] = 0;

    while (!isEmpty(&pq))
    {
        PQNode curr = pop(&pq);
        int u = curr.vertex;
        if (visited[u])
            continue;
        visited[u] = 1;

        Node *temp = city->list[u];
        while (temp)
        {
            int v = temp->dest;
            int newDist = dist[u] + temp->distance;
            if (newDist < dist[v])
            {
                dist[v] = newDist;
                push(&pq, v, newDist);
            }
            temp = temp->next;
        }
    }

    if (dist[dropIndex] == INT_MAX)
    {
        printf("No route found between %s and %s!\n", pickup, drop);
        return 0;
    }

    float baseFare = 30.0;
    float perKmRate = 10.0;
    float totalFare = baseFare + (dist[dropIndex] * perKmRate);

    //printf("\nCalculated Distance: %d km\n", dist[dropIndex]);
    //printf("Fare: %.2f rupees\n", totalFare);

    return totalFare;
}

// Helper: convert time_t to string
void timeToStr(time_t t, char *buffer, size_t bufsize)
{
    if (t == 0)
        snprintf(buffer, bufsize, "N/A");
    else
    {
        struct tm *tm_info = localtime(&t);
        strftime(buffer, bufsize, "%Y-%m-%d %H:%M", tm_info);
    }
}

// CGI-compatible displayAllBills
void displayAllBills()
{
    printf("<div id='view_bills' class='page'>");

    FILE *fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/bills.txt", "rb");
    if (!fp)
    {
        printf("<p class='error-msg'>No bills found!</p>");
        printf("</div>");
        return;
    }

    Ride r;
    int found = 0;

    printf("<h2>All Ride Bills</h2>");
    printf("<table border='1' cellpadding='5' cellspacing='0'>");
    printf("<tr>"
           "<th>Ride ID</th><th>Customer</th><th>Driver</th><th>Pickup</th><th>Drop</th>"
           "<th>Distance</th><th>Fare</th><th>Status</th><th>Booking Time</th><th>Start Time</th><th>End Time</th><th>Download</th>"
           "</tr>");

    while (fread(&r, sizeof(Ride), 1, fp) == 1)
    {
        char booking[20], start[20], end[20];
        timeToStr(r.bookingTime, booking, sizeof(booking));
        timeToStr(r.startTime, start, sizeof(start));
        timeToStr(r.endTime, end, sizeof(end));

        printf("<tr>");
        printf("<td>%d</td><td>%d</td><td>%d</td><td>%s</td><td>%s</td><td>%d km</td>"
               "<td>%.2f</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td>",
               r.rideID, r.customerId, r.driverId, r.pickup, r.drop,
               r.distance, r.fare, r.status, booking, start, end);
        printf("<td><a href='/cgi-bin/admin_dashboard.cgi?action=downloadBill&rideId=%d'>Download</a></td>", r.rideID);
        printf("</tr>");
        found = 1;
    }

    printf("</table>");
    if (!found)
        printf("<p class='error-msg'>No completed ride bills found.</p>");

    fclose(fp);
    printf("</div>");
}

// Generate bill for a completed ride
void generateBill(RideStack *s, int rideId, int custId, int driverId, char pickup[], char drop[], int distance, float fare)
{
    Ride r;
    int found = 0;

    FILE *fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/rides.txt", "rb");
    if (fp)
    {
        Ride temp;
        while (fread(&temp, sizeof(Ride), 1, fp))
        {
            if (temp.rideID == rideId)
            {
                r = temp;
                found = 1;
                break;
            }
        }
        fclose(fp);
    }

    if (!found)
    {
        printf("Content-Type:text/html\r\n\r\n<h3>Ride not found!</h3>");
        return;
    }

    // Recalculate fare if needed
    r.fare = calculateFare(city, r.pickup, r.drop);

    // Push into in-memory stack
    pushStack(s, r);

    Ride *billCopy = (Ride *)malloc(sizeof(Ride));
    *billCopy = r;
    pushBill(billCopy);

    // Append to bills.txt if not already saved
    FILE *bfp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/bills.txt", "rb");
    int duplicate = 0;
    if (bfp)
    {
        Ride temp;
        while (fread(&temp, sizeof(Ride), 1, bfp))
        {
            if (temp.rideID == rideId)
            {
                duplicate = 1;
                break;
            }
        }
        fclose(bfp);
    }

    if (!duplicate)
    {
        bfp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/bills.txt", "ab");
        if (bfp)
        {
            fwrite(&r, sizeof(Ride), 1, bfp);
            fclose(bfp);
        }
    }

    // Calculate totals
    float baseFare = r.fare;
    float gst = baseFare * 0.05;
    float total = baseFare + gst;

    // ---------- Print HTML Bill ----------
   // printf("Content-Type:text/html\r\n\r\n");
    printf("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
    printf("<title>Ride Bill #%d</title>", r.rideID);
    printf("<style>"
           "body{font-family:'Segoe UI',Arial,sans-serif;background:#f4f6f8;margin:0;padding:0;}"
           ".bill-container{max-width:700px;margin:40px auto;background:#fff;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.1);padding:30px;}"
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
}
