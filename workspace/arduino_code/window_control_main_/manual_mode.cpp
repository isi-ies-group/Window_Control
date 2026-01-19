#include "manual_mode.h"
#include "global_structs.h"
#include "movement_task.h"

float manual_increment_mm = 1.0;

void manualMode(const String& dir){


    // ----- PAD ORIGINAL (X / Z GLOBAL) -----
    if (dir == "x_plus")        g_x_val++;
    else if (dir == "x_minus")  g_x_val--;
    else if (dir == "z_plus")   g_z_val++;
    else if (dir == "z_minus")  g_z_val--;

    else {
        Serial.printf("[MANUAL] Unknown dir: %s\n", dir.c_str());
        return;
    }

    //  X/Z  move()
    if (dir.startsWith("x_") || dir.startsWith("z_")) {
        requestMove();
        Serial.printf("[MANUAL] dir=%s x=%.2f z=%.2f\n",
                      dir.c_str(), g_x_val, g_z_val);
    }
}

