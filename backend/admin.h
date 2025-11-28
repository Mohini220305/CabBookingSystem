#ifndef ADMIN_H
#define ADMIN_H


void printHeader();
void printFooter();
void manage_driver(const char *action, const char *idStr, const char *name, const char *phone, const char *vehicle);
void manage_customer(const char *action, const char *idStr);
void viewAllRides();
void displayAllBills();
void showPerformanceInsights();
void downloadReport();
void downloadBill(int rideId);

#endif
