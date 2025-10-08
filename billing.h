#ifndef BILLING_H
#define BILLING_H

#include "ride.h"

#define MAX_BILLS 50

typedef struct
{
    Ride *bills[MAX_BILLS];
    int top;
} BillStack;

extern BillStack billStack;

void initBillStack();
int isBillStackEmpty();
int isBillStackFull();
void pushBill(Ride *ride);
Ride *popBill();
void displayAllBills();
float calculateFare(Graph *city, char pickup[], char drop[]);

#endif