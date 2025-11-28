#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "customer.h"


void printCustomerTable(FILE *fp)
{
    Customer c;
    printf("<h2>All Customers</h2>");
    printf("<table class='data-table'>");
    printf("<tr><th>ID</th><th>Name</th><th>Phone</th><th>Status</th></tr>");
    while (fread(&c, sizeof(Customer), 1, fp) == 1)
    {
        printf("<tr>");
        printf("<td>%d</td>", c.id);
        printf("<td>%s</td>", c.name);
        printf("<td>%s</td>", c.phone);
        printf("<td style='color:%s; font-weight:600;'>%s</td>",
               c.is_blocked ? "red" : "green",
               c.is_blocked ? "Blocked" : "Active");
        printf("</tr>");
    }
    printf("</table>");
}

void printIdFormExternal(const char *externalAction, const char *label, const char *buttonText)
{
    printf("<form method='get' action='/cgi-bin/admin_dashboard.cgi' class='form-box'>");
    printf("<input type='hidden' name='action' value='%s'>", externalAction);
    printf("<label>%s:</label><br>");
    printf("<input type='number' name='id' required placeholder='Enter %s' style='padding:10px;margin-top:8px;border-radius:8px;border:1px solid #ccc;width:100%%;'><br><br>",
           label);
    printf("<input type='submit' class='btn' value='%s'>", buttonText);
    printf("</form>");
}

void manage_customer(const char *action, const char *idStr)
{
    FILE *fp;
    Customer c;

    printf("<div class='page'>");

    if (strcmp(action, "view_all") == 0 || strcmp(action, "view_customers") == 0)
    {
        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt", "rb");
        if (!fp)
        {
            printf("<div class='message-box'><p class='error-msg'>No customers found!</p></div>");
            printf("</div>");
            return;
        }
        printCustomerTable(fp);
        fclose(fp);
    }

    else if (strcmp(action, "remove") == 0)
    {
        if (!idStr || !idStr[0])
        {
            printf("<h2>Remove Customer</h2>");
            printIdFormExternal("remove_customer", "Customer ID", "Remove Customer");
            printf("</div>");
            return;
        }

        int id = atoi(idStr);
        int idFound = 0;

        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt", "rb");
        FILE *temp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/temp.txt", "wb");
        if (!fp || !temp)
        {
            if (fp)
                fclose(fp);
            if (temp)
                fclose(temp);
            printf("<div class='message-box'><p class='error-msg'>File error!</p></div></div>");
            return;
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
            remove("C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt");
            rename("C:/xampp/htdocs/CAB_BOOKING/backend/temp.txt", "C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt");

            printf("<div class='message-box'>");
            printf("<p><strong>Customer removed successfully!</strong></p>");
            printf("<p>Redirecting to <a href='/CAB_BOOKING/frontend/admin_dashboard.html' class='btn'>Admin Dashboard</a>...</p>");
            printf("</div>");
            printf("<meta http-equiv='refresh' content='2;url=/CAB_BOOKING/frontend/admin_dashboard.html'>");
        }
        else
        {
            remove("C:/xampp/htdocs/CAB_BOOKING/backend/temp.txt");
            printf("<div class='message-box'><p class='error-msg'>Customer ID not found!</p></div>");
        }
    }

    else if (strcmp(action, "toggle_block") == 0)
    {
        if (!idStr || !idStr[0])
        {
            printf("<h2>Block / Unblock Customer</h2>");
            printIdFormExternal("block_unblock_customer", "Customer ID", "Block / Unblock Customer");
            printf("</div>");
            return;
        }

        int id = atoi(idStr), found = 0;
        fp = fopen("C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt", "rb+");
        if (!fp)
        {
            printf("<div class='message-box'><p class='error-msg'>No customers found!</p></div></div>");
            return;
        }

        while (fread(&c, sizeof(Customer), 1, fp) == 1)
        {
            if (c.id == id)
            {
                found = 1;
                c.is_blocked = !c.is_blocked;
                fseek(fp, -sizeof(Customer), SEEK_CUR);
                fwrite(&c, sizeof(Customer), 1, fp); 
                break;
            }
        }
        fclose(fp);

        if (found)
        {
            printf("<div class='message-box'>");
            printf("<p><strong>Customer %s successfully!</strong></p>", c.is_blocked ? "blocked" : "unblocked");
            printf("<p>Redirecting to <a href='/CAB_BOOKING/frontend/admin_dashboard.html' class='btn'>Admin Dashboard</a>...</p>");
            printf("</div>");
            printf("<meta http-equiv='refresh' content='2;url=/CAB_BOOKING/frontend/admin_dashboard.html'>");
        }
        else
        {
            printf("<div class='message-box'><p class='error-msg'>Customer ID not found!</p></div>");
        }
    }

    else
    {
        printf("<div class='message-box'><p class='error-msg'>Invalid action!</p></div>");
    }

    printf("</div>"); 
}