#include "manual_mode.h"
#include "global_structs.h"
#include "movement.h"

void manualMode(const String& dir){
    static int test = 0;
    test++;
    Serial.printf("Test = %d\n",test);
    if (dir == "x_plus") g_x_val++;
    else if (dir == "x_minus") g_x_val--;
    else if (dir == "z_plus") g_z_val++;
    else if (dir == "z_minus") g_z_val--;
    move(g_x_val, g_z_val);
    Serial.printf("[MANUAL] dir=%s x=%.2f z=%.2f\n", dir.c_str(), g_x_val, g_z_val);
}

void moveTo(float x, float z){
    move(g_x_val, g_z_val);
}