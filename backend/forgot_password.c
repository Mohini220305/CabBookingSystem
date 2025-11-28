#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "driver.h"
#include "customer.h"

#define DRIVER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/drivers.txt"
#define CUSTOMER_FILE "C:/xampp/htdocs/CAB_BOOKING/backend/customer.txt"
#define TEMP_FILE_DRIVER "C:/xampp/htdocs/CAB_BOOKING/backend/temp_driver.bin"
#define TEMP_FILE_CUSTOMER "C:/xampp/htdocs/CAB_BOOKING/backend/temp_customer.bin"

/* url decode: converts %xx and + to space */
void urldecode(const char *src, char *dst, int dstsize)
{
    int i = 0, j = 0;
    while (src[i] != '\0' && j + 1 < dstsize)
    {
        if (src[i] == '%')
        {
            if (isxdigit((unsigned char)src[i + 1]) && isxdigit((unsigned char)src[i + 2]))
            {
                char hex[3] = {src[i + 1], src[i + 2], '\0'};
                dst[j++] = (char)strtol(hex, NULL, 16);
                i += 3;
            }
            else
            {
                dst[j++] = src[i++];
            }
        }
        else if (src[i] == '+')
        {
            dst[j++] = ' ';
            i++;
        }
        else
        {
            dst[j++] = src[i++];
        }
    }
    dst[j] = '\0';
}

/* getValue: find key=... in query string and copy into out (url-decoded) */
void getValue(const char *query, const char *key, char *out, int outsize)
{
    out[0] = '\0';
    if (!query || !key)
        return;
    size_t klen = strlen(key);
    const char *p = query;
    while ((p = strstr(p, key)) != NULL)
    {
        if (p[klen] == '=')
        {
            const char *val = p + klen + 1;
            const char *amp = strchr(val, '&');
            int len = amp ? (int)(amp - val) : (int)strlen(val);
            char tmp[512];
            if (len >= (int)sizeof(tmp))
                len = (int)sizeof(tmp) - 1;
            strncpy(tmp, val, len);
            tmp[len] = '\0';
            urldecode(tmp, out, outsize);
            return;
        }
        p += klen;
    }
}

/* HTML header/footer with CSS */
void html_header(void)
{
    printf("Content-Type: text/html\r\n\r\n");
    printf("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>"
           "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
           "<title>Forgot Password</title>"
           "<link rel='stylesheet' href='/frontend/login_page.css'>"
           "</head><body>");
}

void html_footer(void)
{
    printf("</body></html>");
}

int main(void)
{
    html_header();

    char *method = getenv("REQUEST_METHOD");
    char raw[4096] = {0};

    if (!method)
    {
        printf("<div class='login-box'><h3 style='color:#c00'>Server error: no request method</h3></div>");
        html_footer();
        return 0;
    }

    if (strcmp(method, "POST") == 0)
    {
        char *lenstr = getenv("CONTENT_LENGTH");
        int len = lenstr ? atoi(lenstr) : 0;
        if (len > 0 && len < (int)sizeof(raw))
        {
            fread(raw, 1, len, stdin);
            raw[len] = '\0';
        }
    }
    else
    { // GET
        char *qs = getenv("QUERY_STRING");
        if (qs)
            strncpy(raw, qs, sizeof(raw) - 1);
    }

    char user_type[64] = "", id_str[32] = "", phone[64] = "";
    char newpass[128] = "", confpass[128] = "";
    getValue(raw, "user_type", user_type, sizeof(user_type));
    getValue(raw, "id", id_str, sizeof(id_str));
    getValue(raw, "phone", phone, sizeof(phone));
    getValue(raw, "new_password", newpass, sizeof(newpass));
    getValue(raw, "confirm_password", confpass, sizeof(confpass));

    int id = id_str[0] ? atoi(id_str) : -1;
    int is_reset_stage = (newpass[0] != '\0');

    if (is_reset_stage)
    {
        /* ----- Reset password stage ----- */
        if (!user_type[0] || id < 0 || !newpass[0] || !confpass[0])
        {
            printf("<div class='login-box'><h3 style='color:#c00'>Missing data. Please try again.</h3>"
                   "<a href='/CAB_BOOKING/frontend/forgot_password.html' class='back-link'>Back</a></div>");
            html_footer();
            return 0;
        }

        if (strcmp(newpass, confpass) != 0)
        {
            printf("<div class='login-box'><h3 style='color:#c00'>Passwords do not match.</h3>"
                   "<a href='/CAB_BOOKING/frontend/forgot_password.html' class='back-link'>Start over</a></div>");
            html_footer();
            return 0;
        }

        const char *origFile = NULL;
        const char *tempFile = NULL;
        int updated = 0;

        if (strcmp(user_type, "driver") == 0)
        {
            origFile = DRIVER_FILE;
            tempFile = TEMP_FILE_DRIVER;
            FILE *fin = fopen(origFile, "rb");
            FILE *fout = fopen(tempFile, "wb");
            if (!fin || !fout)
            {
                if (fin)
                    fclose(fin);
                if (fout)
                    fclose(fout);
                printf("<div class='login-box'><h3 style='color:#c00'>File access error.</h3></div>");
                html_footer();
                return 0;
            }
            Driver d;
            while (fread(&d, sizeof(Driver), 1, fin) == 1)
            {
                if (d.id == id)
                {
                    memset(d.pass, 0, sizeof(d.pass));
                    strncpy(d.pass, newpass, sizeof(d.pass) - 1);
                    d.mustChangePassword = 0;
                    updated = 1;
                }
                fwrite(&d, sizeof(Driver), 1, fout);
            }
            fclose(fin);
            fclose(fout);
            if (updated)
            {
                remove(origFile);
                rename(tempFile, origFile);
                printf("<div class='login-box'><h2 style='color:green'>Password reset successful!</h2>"
                       "<a href='/CAB_BOOKING/frontend/driver_login.html' class='btn'>Go to Driver Login</a></div>");
            }
            else
            {
                remove(tempFile);
                printf("<div class='login-box'><h3 style='color:#c00'>Driver not found. Try again.</h3>"
                       "<a href='/CAB_BOOKING/frontend/forgot_password.html' class='back-link'>Back</a></div>");
            }
        }
        else if (strcmp(user_type, "customer") == 0)
        {
            origFile = CUSTOMER_FILE;
            tempFile = TEMP_FILE_CUSTOMER;
            FILE *fin = fopen(origFile, "rb");
            FILE *fout = fopen(tempFile, "wb");
            if (!fin || !fout)
            {
                if (fin)
                    fclose(fin);
                if (fout)
                    fclose(fout);
                printf("<div class='login-box'><h3 style='color:#c00'>File access error.</h3></div>");
                html_footer();
                return 0;
            }
            Customer c;
            while (fread(&c, sizeof(Customer), 1, fin) == 1)
            {
                if (c.id == id)
                {
                    memset(c.password, 0, sizeof(c.password));
                    strncpy(c.password, newpass, sizeof(c.password) - 1);
                    updated = 1;
                }
                fwrite(&c, sizeof(Customer), 1, fout);
            }
            fclose(fin);
            fclose(fout);
            if (updated)
            {
                remove(origFile);
                rename(tempFile, origFile);
                printf("<div class='login-box'><h2 style='color:green'>Password reset successful!</h2>"
                       "<a href='/CAB_BOOKING/frontend/customer_login.html' class='btn'>Go to Customer Login</a></div>");
            }
            else
            {
                remove(tempFile);
                printf("<div class='login-box'><h3 style='color:#c00'>Customer not found. Try again.</h3>"
                       "<a href='/CAB_BOOKING/frontend/forgot_password.html' class='back-link'>Back</a></div>");
            }
        }
        else
        {
            printf("<div class='login-box'><h3 style='color:#c00'>Invalid user type.</h3></div>");
        }
    }
    else
    {
        /* ----- Verify stage ----- */
        if (!user_type[0] || id < 0 || !phone[0])
        {
            printf("<div class='login-box'><h3 style='color:#c00'>Please provide user type, ID and registered phone.</h3>"
                   "<a href='/CAB_BOOKING/frontend/forgot_password.html' class='back-link'>Back</a></div>");
            html_footer();
            return 0;
        }

        int found = 0;
        if (strcmp(user_type, "driver") == 0)
        {
            FILE *fin = fopen(DRIVER_FILE, "rb");
            if (!fin)
            {
                printf("<div class='login-box'><h3 style='color:#c00'>Driver file not accessible.</h3></div>");
                html_footer();
                return 0;
            }
            Driver d;
            while (fread(&d, sizeof(Driver), 1, fin) == 1)
            {
                if (d.id == id && strncmp(d.phone, phone, sizeof(d.phone)) == 0)
                {
                    found = 1;
                    break;
                }
            }
            fclose(fin);
        }
        else if (strcmp(user_type, "customer") == 0)
        {
            FILE *fin = fopen(CUSTOMER_FILE, "rb");
            if (!fin)
            {
                printf("<div class='login-box'><h3 style='color:#c00'>Customer file not accessible.</h3></div>");
                html_footer();
                return 0;
            }
            Customer c;
            while (fread(&c, sizeof(Customer), 1, fin) == 1)
            {
                if (c.id == id && strncmp(c.phone, phone, sizeof(c.phone)) == 0)
                {
                    found = 1;
                    break;
                }
            }
            fclose(fin);
        }
        else
        {
            printf("<div class='login-box'><h3 style='color:#c00'>Invalid user type.</h3></div>");
            html_footer();
            return 0;
        }

        if (found)
        {
            printf("<div class='login-box'><h2>Verification successful ✅</h2>");
            printf("<form method='post' action='/cgi-bin/forgot_password.cgi'>");
            printf("<input type='hidden' name='user_type' value='%s'>", user_type);
            printf("<input type='hidden' name='id' value='%d'>", id);
            printf("<input type='password' name='new_password' placeholder='New password' required>");
            printf("<input type='password' name='confirm_password' placeholder='Confirm password' required>");
            printf("<button type='submit' class='btn'>Reset Password</button>");
            printf("</form></div>");
        }
        else
        {
            printf("<div class='login-box'><h3 style='color:#c00'>ID and phone did not match our records.</h3>"
                   "<a href='/CAB_BOOKING/frontend/forgot_password.html' class='back-link'>Try again</a></div>");
        }
    }

    html_footer();
    return 0;
}
