#ifndef global_structs_h
#define global_structs_h


#include <Arduino.h>
#include "global_def.h"

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
    float AOIt;
    float AOIl;
} AOIInputs;

typedef struct {
    float AOIt;
    float AOIl;
    const float (*matrix_X)[N];
    const float (*matrix_Z)[N];
} InterpolInputs;


extern SPAInputs g_SPAInputs;
extern AOIInputs g_AOIInputs;
extern InterpolInputs g_InterpolInputs;

extern SemaphoreHandle_t sem_SPA_AOI;
extern SemaphoreHandle_t sem_AOI_Inter;


extern String g_country;

extern float g_x_val;
extern float g_z_val;


#endif