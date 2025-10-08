#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "map.h"
#include "ride.h"

typedef struct
{
    int id;
    char name[50];
    char phone[15];
    char password[20];
    int is_blocked; // 1 for blocked
} Customer;

void customerMenu(Graph *city, int custId);

#endif