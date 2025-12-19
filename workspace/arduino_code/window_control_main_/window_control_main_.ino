#include <Arduino.h>
#include "autoMode.h"
#include "gps.h"
#include "web_server.h"
#include "storage.h"
#include "state_machine.h"
#include "movement.h"

#define DEBUG 1  // 0 to unable _print(pt)

#if DEBUG
  #define _print(pt)  Serial.print(#pt " = "); Serial.println(pt, 6);
#else
  #define _print(pt)
#endif





void setup() {
  Serial.begin(115200);
  initFSM();
  loadData();
  gpsInit();
  serverInit();
  setLocalTime();
  init_motors();
}

void loop() {
  //delay(10000);
  runMachine();
  //printLocalTime(); 
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
