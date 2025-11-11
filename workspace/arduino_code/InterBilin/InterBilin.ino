#include <Arduino.h>
#include "autoMode.h"
#include "gps.h"
#include "web_server.h"
#include "storage.h"


#define N 86
#define DEBUG 1  // 0 to unable _print(pt)

#if DEBUG
  #define _print(pt)  Serial.print(#pt " = "); Serial.println(pt, 6);
#else
  #define _print(pt)
#endif


double lon = -3.7038;
double pan = 0;
double tilt = 0;
bool tilt_correction = false;



void setup() {
  Serial.begin(115200);
  gpsInit();
  serverInit();
  loadData();
  setLocalTime();
}

void loop() {
  delay(10000);
  printLocalTime(); 
}
