#ifndef global_structs_h
#define global_structs_h

#include "global_def.h"
#include <time.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    double longitude;
    double latitude;
} SPAInputs;

typedef struct {
    double pan;
    double azimuth;
    double elevation;
    double tilt;
    bool tilt_correction;
} AOIInputs;

typedef struct {
    float AOIt;
    float AOIl;
    const float (*matrix_X)[MATRIX_SIZE];
    const float (*matrix_Z)[MATRIX_SIZE];
} InterpolInputs;




extern SPAInputs g_SPAInputs;
extern AOIInputs g_AOIInputs;
extern InterpolInputs g_InterpolInputs;

extern int auto_counter;
extern char g_country[32];

extern float g_x_val;
extern float g_z_val;
extern bool auto_on;
extern bool manual_time;

extern time_t g_sunrise_epoch;
extern time_t g_sunset_epoch;
//extern bool g_auto_mode_on;
//extern bool use_simulated_time;

#endif
