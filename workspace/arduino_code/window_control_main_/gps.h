#ifndef gps_h
#define gps_h


void printLocalTime();
void gpsInit();
void setSystemTimeFromGPS();
void setLocalTime();
void debugGPS();
bool syncTimeFromGPSQuick();
void setSystemTimeManualLocal(int year, int month, int day, int hour, int min, int sec);

#endif
