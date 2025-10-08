#include "stack.h"
#include <stdio.h>

RideStack rs = {.top = -1};

void initStack(RideStack *s){
    s->top = -1;
}

int isEmptyStack(RideStack *s){
    return s->top == -1;
}

int isFullStack(RideStack *s){
    return s->top == MAX - 1;
}

void pushStack(RideStack *s, Ride r){
    if (!isFullStack(s))
        s->rides[++s->top] = r;
    else
        printf("Stack full! Cannot push ride.\n");
}
