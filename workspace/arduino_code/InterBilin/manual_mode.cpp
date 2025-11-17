#include "manual_mode.h"
#include "global_structs.h" 

void manualMode(const String& dir){
    if (dir == "x_plus") {
        g_x_val++;
        Serial.printf("[MANUAL] X+   g_x_val = %d\n", g_x_val);
    }
    else if (dir == "x_minus") {
        g_x_val--;
        Serial.printf("[MANUAL] X-   g_x_val = %d\n", g_x_val);
    }
    else if (dir == "z_plus") {
        g_z_val++;
        Serial.printf("[MANUAL] Z+   g_z_val = %d\n", g_z_val);
    }
    else if (dir == "z_minus") {
        g_z_val--;
        Serial.printf("[MANUAL] Z-   g_z_val = %d\n", g_z_val);
    }
    else {
        Serial.printf("[MANUAL] Unknown dir: %s\n", dir.c_str());
    }	
}