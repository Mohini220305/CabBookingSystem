#ifndef RIDE_H
#define RIDE_H

#include <time.h>
#include "map.h"
#include "customer.h"
#include "driver.h"

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
    time_t bookingTime;      // when ride is booked
    time_t cancellationTime; // if cancelled
    time_t startTime;        // when ride starts
    time_t endTime;          // when ride ends
    struct Ride *next;
} Ride;

extern int rideID;
extern Ride *front;
extern Ride *rear;
extern Graph *city;

void printTime(time_t t);
Ride *bookRide(Graph *city, int customerId, char pickup[], char drop[]);
int allocateDriver(Graph *city, Driver drivers[], int numDrivers, int pickupLoc, char *pickup, char *drop, int excludeDriverId);
void cancelRide(int rideID);
void enqueueRide(Ride *newRide);
//void cancelRide(int custId, int rideId);
void reassignRide(Ride *ride, Graph *city, int driverId);
void saveRideToFile(Ride *r);
void loadRidesFromFile();
void viewRideStatus(int custID); // for customer
void showCustomerHistory(int customerId);

#endif
