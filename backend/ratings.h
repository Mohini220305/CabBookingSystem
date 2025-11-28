#ifndef RATING_H
#define RATING_H


// Structure to store each rating
typedef struct
{
    int rideId;
    int custId;
    int driverID;
    float rating;
} Rating;

// Function declarations

// Add a new rating for a driver
void addRating(int rideId, int custId, int driverID, float rating);
int hasRated(int rideId, int custId);
// Calculate the average rating of a driver
float getAverageRating(int driverID);

// Display all drivers with their average rating (admin view)
void viewDriverRatings();

#endif // RATING_H
