#ifndef DRIVER_H
#define DRIVER_H

typedef struct Driver
{
    int id;
    char name[50];
    char vehicle[20];
    char phone[15];
    char pass[20];
    int location;  // index of location
    int available; // 1 if free, 0 if busy
    int mustChangePassword;
} Driver;

void driverMenu(int driverId);
void viewAssignedRides(int driverId);
void viewCompletedRides(int driverId);
int loadDrivers(Driver drivers[]);
void updateDriverLocation(int driverId);

#endif