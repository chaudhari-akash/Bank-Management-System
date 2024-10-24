#include <stdio.h>
#include <time.h>

int main() {
    // Create a buffer to hold the formatted date and time
    char datetime[20];

    // Get the current time
    time_t now = time(NULL);

    // Convert to local time format
    struct tm *local = localtime(&now);

    // Format the time and date as a string (YYYY-MM-DD HH:MM:SS)
    strftime(datetime, sizeof(datetime), "%d-%m-%Y %H:%M", local);

    // Print the formatted string
    printf("Current time: %s\n", datetime);

    return 0;
}
