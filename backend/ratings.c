#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ratings.h"
#include "driver.h"

#define RATINGS_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/ratings.txt"

// Function to add a rating
void addRating(int rideId, int custId, int driverID, float rating)
{
    FILE *fp = fopen(RATINGS_FILE, "ab"); // append in binary
    if (!fp)
    {
        printf("Content-type:text/html\n\n");
        printf("Error: Cannot open ratings file!");
        return;
    }

    Rating r;
    r.rideId = rideId;
    r.custId = custId;
    r.driverID = driverID;
    r.rating = rating;

    fwrite(&r, sizeof(Rating), 1, fp);
    fclose(fp);

    printf("<p>Rating submitted successfully!</p>");
}
int hasRated(int rideId, int custId)
{
    FILE *fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/ratings.txt", "rb");
    if (!fp)
        return 0;

    Rating r;
    while (fread(&r, sizeof(Rating), 1, fp) == 1)
    {
        if (r.rideId == rideId && r.custId == custId)
        {
            fclose(fp);
            return 1; // Already rated
        }
    }
    fclose(fp);
    return 0; // Not rated yet
}

// Function to calculate average rating for a driver
float getAverageRating(int driverID)
{
    FILE *fp = fopen(RATINGS_FILE, "rb");
    if (!fp)
        return 0.0;

    Rating r;
    float total = 0.0;
    int count = 0;

    while (fread(&r, sizeof(Rating), 1, fp) == 1)
    {
        if (r.driverID == driverID)
        {
            total += r.rating;
            count++;
        }
    }
    fclose(fp);

    if (count == 0)
        return 0.0;
    return total / count;
}

/*// Function to extract parameters from query string
void getParam(char *query, const char *key, char *value)
{
    char *p = strstr(query, key);
    if (p)
    {
        p += strlen(key) + 1; // skip key=
        int i = 0;
        while (*p && *p != '&')
            value[i++] = *p++;
        value[i] = '\0';
    }
}
*/
void viewDriverRatings()
{
    FILE *fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt", "rb");
    if (!fp)
    {
        printf("<p>No drivers found!</p>");
        return;
    }

    Driver d;
    printf("<h2>Driver Ratings</h2>");
    printf("<table border='1' cellpadding='5' cellspacing='0'>");
    printf("<tr><th>Driver ID</th><th>Name</th><th>Average Rating</th></tr>");

    while (fread(&d, sizeof(Driver), 1, fp) == 1)
    {
        float avg = getAverageRating(d.id);
        printf("<tr><td>%d</td><td>%s</td><td>%.2f</td></tr>", d.id, d.name, avg);
    }

    printf("</table>");
    printf("<br><a href='admin_dashboard.cgi?action=dashboard'>Back to Dashboard</a>");
    fclose(fp);
}

/*int main()
{
    printf("Content-type:text/html\n\n");
    printf("<html><head><title>Rate Driver</title></head><body>");
    printf("<h2>Rate Your Driver</h2>");

    char *query = getenv("QUERY_STRING");
    if (!query || strlen(query) == 0)
    {
        // Show form if no query
        printf("<form action=\"/cgi-bin/rate_driver.cgi\" method=\"get\">");
        printf("Driver ID: <input type=\"number\" name=\"driverID\" required><br><br>");
        printf("Rating (1-5): <input type=\"number\" name=\"rating\" min=\"1\" max=\"5\" step=\"0.1\" required><br><br>");
        printf("<input type=\"submit\" value=\"Submit Rating\">");
        printf("</form>");
    }
    else
    {
        // Process rating
        char idStr[10], ratingStr[10];
        getParam(query, "driverID", idStr);
        getParam(query, "rating", ratingStr);

        int driverID = atoi(idStr);
        float rating = atof(ratingStr);

        addRating(driverID, rating);

        float avg = getAverageRating(driverID);
        printf("<p>Current average rating for driver %d: %.2f</p>", driverID, avg);
        printf("<a href=\"/cgi-bin/rate_driver.cgi\">Rate another driver</a>");
    }

    printf("</body></html>");
    return 0;
}
*/