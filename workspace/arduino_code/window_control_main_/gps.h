#ifndef gps_h
#define gps_h


void printLocalTime();
void gpsInit();
void setSystemTimeFromGPS();
void setLocalTime();
void debugGPS();
bool syncTimeFromGPSQuick();


#endif
