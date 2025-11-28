#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "driver.h"

// ---------- Helper: URL decode ----------
void urlDecoder(char *dst, const char *src)
{
    char a, b;
    while (*src)
    {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b)))
        {
            if (a >= 'a')
                a -= 'a' - 'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a' - 'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16 * a + b;
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

// ---------- Helper: Print driver table ----------
void printDriverTable(FILE *fp)
{
    Driver d;
    printf("<h2>All Drivers</h2>");
    printf("<table border='1' cellpadding='5' cellspacing='0'>");
    printf("<tr><th>ID</th><th>Name</th><th>Phone</th><th>Vehicle</th></tr>");
    while (fread(&d, sizeof(Driver), 1, fp))
    {
        printf("<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td></tr>", d.id, d.name, d.phone, d.vehicle);
    }
    printf("</table>");
}

// ---------- Helper: Print HTML form ----------
void printDriverForm(const char *action, const char *idVal, const char *nameVal, const char *phoneVal, const char *vehicleVal)
{
    printf("<form method='get' action='/cgi-bin/admin_dashboard.cgi'>");
    printf("<input type='hidden' name='action' value='%s'>", action);
    if (idVal)
        printf("Driver ID: <input type='number' name='id' value='%s' required><br><br>", idVal);
    if (nameVal)
        printf("Name: <input type='text' name='name' value='%s'><br><br>", nameVal);
    if (phoneVal)
        printf("Phone: <input type='text' name='phone' value='%s'><br><br>", phoneVal);
    if (vehicleVal)
        printf("Vehicle: <input type='text' name='vehicle' value='%s'><br><br>", vehicleVal);
    printf("<input type='submit' value='%s'>", strcmp(action, "add_driver") == 0 ? "Add Driver" : strcmp(action, "modify_driver") == 0 ? "Modify Driver"
                                                                                                                                       : "Submit");
    printf("</form>");
}
void manage_driver(const char *action, const char *idStr, const char *nameRaw, const char *phoneRaw, const char *vehicleRaw)
{
    FILE *fp;
    Driver d;
    char name[50], phone[15], vehicle[20];

    if (nameRaw)
        urlDecoder(name, nameRaw);
    else
        name[0] = '\0';
    if (phoneRaw)
        urlDecoder(phone, phoneRaw);
    else
        phone[0] = '\0';
    if (vehicleRaw)
        urlDecoder(vehicle, vehicleRaw);
    else
        vehicle[0] = '\0';

    printf("<div class='content'>");
    printf("<div id='%s' class='page'>", action);

    // ---------- VIEW DRIVER ----------
    if (strcmp(action, "view_driver") == 0)
    {
        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt", "rb");
        if (!fp)
        {
            printf("<div class='message-box error-msg'>No drivers found!</div>");
            printf("</div></div>");
            return;
        }

        printf("<h2 class='section-title'>All Drivers</h2>");
        printf("<table class='styled-table'>");
        printf("<thead><tr><th>ID</th><th>Name</th><th>Phone</th><th>Vehicle</th></tr></thead><tbody>");

        while (fread(&d, sizeof(Driver), 1, fp))
        {
            printf("<tr><td>%d</td><td>%s</td><td>%s</td><td>%s</td></tr>", d.id, d.name, d.phone, d.vehicle);
        }

        printf("</tbody></table>");
        fclose(fp);
    }

    // ---------- ADD DRIVER ----------
    else if (strcmp(action, "add_driver") == 0)
    {
        if (!name[0] || !phone[0] || !vehicle[0])
        {
            printf("<h2 class='section-title'>Add New Driver</h2>");
            printf("<form class='styled-form' method='get' action='/cgi-bin/admin_dashboard.cgi'>");
            printf("<input type='hidden' name='action' value='add_driver'>");
            printf("<label>Name:</label><input type='text' name='name' required><br>");
            printf("<label>Phone:</label><input type='text' name='phone' required><br>");
            printf("<label>Vehicle:</label><input type='text' name='vehicle' required><br>");
            printf("<button type='submit' class='btn-primary'>Add Driver</button>");
            printf("</form>");
            printf("</div></div>");
            return;
        }

        int newId = 1;
        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt", "rb");
        if (fp)
        {
            Driver temp;
            while (fread(&temp, sizeof(Driver), 1, fp))
                if (temp.id >= newId)
                    newId = temp.id + 1;
            fclose(fp);
        }

        d.id = newId;
        strcpy(d.name, name);
        strcpy(d.phone, phone);
        strcpy(d.vehicle, vehicle);
        strcpy(d.pass, "driver123");
        d.available = 1;
        d.location = 1;
        d.completedRides = 0;
        d.earnings = 0.0;
        d.mustChangePassword = 1;

        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt", "ab");
        if (!fp)
        {
            printf("<div class='message-box error-msg'>Error opening file!</div></div></div>");
            return;
        }
        fwrite(&d, sizeof(Driver), 1, fp);
        fclose(fp);

        printf("<div class='message-box success-msg'>Driver added successfully! ID: %d</div>", d.id);
        printf("<meta http-equiv='refresh' content='2;url=/CAB_BOOKING/frontend/admin_dashboard.html'>");
    }

    // ---------- DELETE DRIVER ----------
    else if (strcmp(action, "delete_driver") == 0)
    {
        if (!idStr || !idStr[0])
        {
            printf("<h2 class='section-title'>Delete Driver</h2>");
            printf("<form class='styled-form' method='get' action='/cgi-bin/admin_dashboard.cgi'>");
            printf("<input type='hidden' name='action' value='delete_driver'>");
            printf("<label>Driver ID:</label><input type='number' name='id' required><br>");
            printf("<button type='submit' class='btn-danger'>Delete Driver</button>");
            printf("</form>");
            printf("</div></div>");
            return;
        }

        int id = atoi(idStr), found = 0;
        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt", "rb");
        FILE *temp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/temp.txt", "wb");
        if (!fp || !temp)
        {
            printf("<div class='message-box error-msg'>File error!</div></div></div>");
            if (fp)
                fclose(fp);
            if (temp)
                fclose(temp);
            return;
        }

        while (fread(&d, sizeof(Driver), 1, fp))
        {
            if (d.id == id)
            {
                found = 1;
                continue;
            }
            fwrite(&d, sizeof(Driver), 1, temp);
        }
        fclose(fp);
        fclose(temp);

        if (found)
        {
            remove("C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt");
            rename("C:/xampp/htdocs/CAB_BOOKING/backend/temp.txt", "C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt");
            printf("<div class='message-box success-msg'>Driver removed successfully!</div>");
            printf("<meta http-equiv='refresh' content='2;url=/CAB_BOOKING/frontend/admin_dashboard.html'>");
        }
        else
        {
            remove("C:/xampp/htdocs/CAB_BOOKING/backend/temp.txt");
            printf("<div class='message-box error-msg'>Driver ID not found!</div>");
        }
    }

    // ---------- MODIFY DRIVER ----------
    else if (strcmp(action, "modify_driver") == 0)
    {
        if (!idStr || !idStr[0])
        {
            printf("<h2 class='section-title'>Modify Driver</h2>");
            printf("<form class='styled-form' method='get' action='/cgi-bin/admin_dashboard.cgi'>");
            printf("<input type='hidden' name='action' value='modify_driver'>");
            printf("<label>Driver ID:</label><input type='number' name='id' required><br>");
            printf("<label>New Name:</label><input type='text' name='name'><br>");
            printf("<label>New Phone:</label><input type='text' name='phone'><br>");
            printf("<label>New Vehicle:</label><input type='text' name='vehicle'><br>");
            printf("<button type='submit' class='btn-primary'>Update</button>");
            printf("</form>");
            printf("</div></div>");
            return;
        }

        int id = atoi(idStr), found = 0;
        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt", "rb+");
        if (!fp)
        {
            printf("<div class='message-box error-msg'>No drivers found!</div></div></div>");
            return;
        }

        while (fread(&d, sizeof(Driver), 1, fp))
        {
            if (d.id == id)
            {
                found = 1;
                if (name[0])
                    strcpy(d.name, name);
                if (phone[0])
                    strcpy(d.phone, phone);
                if (vehicle[0])
                    strcpy(d.vehicle, vehicle);
                fseek(fp, -sizeof(Driver), SEEK_CUR);
                fwrite(&d, sizeof(Driver), 1, fp);
                break;
            }
        }
        fclose(fp);

        if (found)
        {
            printf("<div class='message-box success-msg'>Driver details updated successfully!</div>");
            printf("<meta http-equiv='refresh' content='2;url=/CAB_BOOKING/frontend/admin_dashboard.html'>");
        }
        else
        {
            printf("<div class='message-box error-msg'>Driver ID not found!</div>");
        }
    }
    else
    {
        printf("<div class='message-box error-msg'>Invalid action!</div>");
    }

    printf("</div></div>");
}
