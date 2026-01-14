#ifndef manual_mode_h
#define manual_mode_h

#include <Arduino.h>


void manualMode(const String& dir);
void moveTo(float x, float z);
void move_mxle(const String& dir);
void move_mxli(const String& dir);
void move_mxre(const String& dir);
void move_mxri(const String& dir);
void move_mzl(const String& dir);
void move_mzr(const String& dir);



#endif
