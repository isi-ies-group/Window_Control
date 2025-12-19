#include "manual_mode.h"
#include "global_structs.h"
#include "movement.h"

void manualMode(const String& dir){
    // static int test = 0;
    // test++;
    // Serial.printf("Test = %d\n",test);
    // if (dir == "x_plus") g_x_val++;
    // else if (dir == "x_minus") g_x_val--;
    // else if (dir == "z_plus") g_z_val++;
    // else if (dir == "z_minus") g_z_val--;
    // move(g_x_val, g_z_val);
    // Serial.printf("[MANUAL] dir=%s x=%.2f z=%.2f\n", dir.c_str(), g_x_val, g_z_val);

    // ----- PAD ORIGINAL (X / Z GLOBAL) -----
    if (dir == "x_plus")        g_x_val++;
    else if (dir == "x_minus")  g_x_val--;
    else if (dir == "z_plus")   g_z_val++;
    else if (dir == "z_minus")  g_z_val--;

    // ----- MXL EXTERIOR / INTERIOR -----
    else if (dir == "mxle_plus")  move_mxle("x_plus");
    else if (dir == "mxle_minus") move_mxle("x_minus");
    else if (dir == "mxli_plus")  move_mxli("x_plus");
    else if (dir == "mxli_minus") move_mxli("x_minus");

    // ----- MXR EXTERIOR / INTERIOR -----
    else if (dir == "mxre_plus")  move_mxre("x_plus");
    else if (dir == "mxre_minus") move_mxre("x_minus");
    else if (dir == "mxri_plus")  move_mxri("x_plus");
    else if (dir == "mxri_minus") move_mxri("x_minus");

    // ----- MZ LEFT / RIGHT -----
    else if (dir == "mzl_plus")   move_mzl("z_plus");
    else if (dir == "mzl_minus")  move_mzl("z_minus");
    else if (dir == "mzr_plus")   move_mzr("z_plus");
    else if (dir == "mzr_minus")  move_mzr("z_minus");

    else {
        Serial.printf("[MANUAL] Unknown dir: %s\n", dir.c_str());
        return;
    }

    //  X/Z  move()
    if (dir.startsWith("x_") || dir.startsWith("z_")) {
        move(g_x_val, g_z_val);
        Serial.printf("[MANUAL] dir=%s x=%.2f z=%.2f\n",
                      dir.c_str(), g_x_val, g_z_val);
    }
}


void moveTo(float x, float z){
    move(g_x_val, g_z_val);
}

void move_mxle(const String& dir){
    if (dir == "x_plus") x_mxle++;
    else if (dir == "x_minus") x_mxle--;
    move_external_vertical_left(x_mxle);
    Serial.printf("[MANUAL] dir=%s x=%.2f", dir.c_str(), x_mxle);
}


void move_mxli(const String& dir){
    if (dir == "x_plus") x_mxli++;
    else if (dir == "x_minus") x_mxli--;
    move_internal_vertical_left(x_mxli);
    Serial.printf("[MANUAL] dir=%s x=%.2f", dir.c_str(), x_mxli);
}
void move_mxre(const String& dir){
    if (dir == "x_plus") x_mxre++;
    else if (dir == "x_minus") x_mxre--;
    move_external_vertical_right(x_mxre);
    Serial.printf("[MANUAL] dir=%s x=%.2f", dir.c_str(), x_mxre);
}
void move_mxri(const String& dir){
    if (dir == "x_plus") x_mxri++;
    else if (dir == "x_minus") x_mxri--;
    move_internal_vertical_right(x_mxri);
    Serial.printf("[MANUAL] dir=%s x=%.2f", dir.c_str(), x_mxri);    
}

void move_mzl(const String& dir){
    if (dir == "z_plus") z_mzl++;
    else if (dir == "z_minus") z_mzl--;
    move_horizontal_left(z_mzl);
    Serial.printf("[MANUAL] dir=%s z=%.2f\n", dir.c_str(), z_mzl);
}
void move_mzr(const String& dir){
    if (dir == "z_plus") z_mzr++;
    else if (dir == "z_minus") z_mzr--;
    move_horizontal_right(z_mzr);
    Serial.printf("[MANUAL] dir=%s z=%.2f\n", dir.c_str(), z_mzr);

}
