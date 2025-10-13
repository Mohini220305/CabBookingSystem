#ifndef STACK_H
#define STACK_H

#include "ride.h"

#define MAX 100

typedef struct RideStack
{
    Ride rides[MAX];
    int top;
} RideStack;

extern RideStack rs;

void initStack(RideStack *s);
int isEmptyStack(RideStack *s);
int isFullStack(RideStack *s);
void pushStack(RideStack *s, Ride r);
void generateBill(RideStack *s,int rideId, int custId, int driverId, char pickup[], char drop[], int distance, float fare);

#endif
