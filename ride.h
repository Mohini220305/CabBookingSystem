#ifndef RIDE_H
#define RIDE_H

#include "customer.h"
#include "driver.h"
#include "map.h"

typedef struct Ride
{
    int rideID;
    int customerId;
    int driverId;
    char pickup[50];
    char drop[50];
    int distance;
    float fare;
    char status[30]; // "Waiting","Pending","Completed","Cancelled"
    struct Ride *next;
} Ride;

extern int rideID;
extern Ride *front;
extern Ride *rear;
extern Graph *city;

Ride *bookRide(Graph *city, int customerId, char pickup[], char drop[]);
int allocateDriver(Graph *city, Driver drivers[], int numDrivers, int pickupLoc);
void cancelRide(int rideID);
void reassignRide(Ride *ride, Graph *city);
void saveRideToFile(Ride *r);
void loadRidesFromFile();
void viewRideStatus(int custID); // for customer
void showCustomerHistory(int customerId);
void showDriverHistory(int driverId);
//float calculateFare(int distance);

#endif
