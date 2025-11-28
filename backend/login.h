#ifndef LOGIN_H
#define LOGIN_H

#include "customer.h"
#include "admin.h"
#include "driver.h"

int adminLogin(char *query);
int driverLogin(char *query);
int customerLogin(char *query);
void registerCustomer(char *query);

char *getParam(char *query, const char *key, char *value);

#endif
