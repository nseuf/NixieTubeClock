#ifndef INC_UTC_H_
#define INC_UTC_H_

#include <main.h>

// Structure to represent time offset in hours and minutes
typedef struct {
    int hours;
    int minutes;
} TimeOffset;

// Function to calculate the time offset from the UTC constant using switch
static inline TimeOffset calculate_utc_offset(int utc_constant) {
    TimeOffset offset = {0, 0}; // Default offset

    switch (utc_constant) {
        case 1:  offset.hours = -12; break;           // UTC−12:00
        case 2:  offset.hours = -11; break;           // UTC−11:00
        case 3:  offset.hours = -10; break;           // UTC−10:00
        case 4:  offset.hours = -9; offset.minutes = -30; break; 		  // UTC−09:30
        case 5:  offset.hours = -9; break;            // UTC−09:00
        case 6:  offset.hours = -8; break;            // UTC−08:00
        case 7:  offset.hours = -7; break;            // UTC−07:00
        case 8:  offset.hours = -6; break;            // UTC−06:00
        case 9:  offset.hours = -5; break;            // UTC−05:00
        case 10: offset.hours = -4; break;            // UTC−04:00
        case 11: offset.hours = -3; offset.minutes = -30; break; 		  // UTC−03:30
        case 12: offset.hours = -3; break;            // UTC−03:00
        case 13: offset.hours = -2; break;            // UTC−02:00
        case 14: offset.hours = -1; break;            // UTC−01:00
        case 15: offset.hours = 0; break;             // UTC+00:00
        case 16: offset.hours = 1; break;             // UTC+01:00
        case 17: offset.hours = 2; break;             // UTC+02:00
        case 18: offset.hours = 3; break;             // UTC+03:00
        case 19: offset.hours = 3; offset.minutes = 30; break;			  // UTC+03:30
        case 20: offset.hours = 4; break;             // UTC+04:00
        case 21: offset.hours = 4; offset.minutes = 30; break;			  // UTC+04:30
        case 22: offset.hours = 5; break;             // UTC+05:00
        case 23: offset.hours = 5; offset.minutes = 30; break;			  // UTC+05:30
        case 24: offset.hours = 5; offset.minutes = 45; break;			  // UTC+05:45
        case 25: offset.hours = 6; break;             // UTC+06:00
        case 26: offset.hours = 6; offset.minutes = 30; break;			  // UTC+06:30
        case 27: offset.hours = 7; break;             // UTC+07:00
        case 28: offset.hours = 8; break;             // UTC+08:00
        case 29: offset.hours = 8; offset.minutes = 45; break;			  // UTC+08:45
        case 30: offset.hours = 9; break;             // UTC+09:00
        case 31: offset.hours = 9; offset.minutes = 30; break;			  // UTC+09:30
        case 32: offset.hours = 10; break;            // UTC+10:00
        case 33: offset.hours = 10; offset.minutes = 30; break;			  // UTC+10:30
        case 34: offset.hours = 11; break;            // UTC+11:00
        case 35: offset.hours = 12; break;            // UTC+12:00
        case 36: offset.hours = 12; offset.minutes = 45; break; 		  // UTC+12:45
        case 37: offset.hours = 13; break;            // UTC+13:00
        case 38: offset.hours = 14; break;            // UTC+14:00
        default:
            // Invalid UTC constant
            offset.hours = 0;
            offset.minutes = 0;
            break;
    }

    return offset;
}





#endif /* INC_UTC_H_ */
