#include <Arduino.h>
#include "movement.h"
#include "manual_mode.h"

// ---------------- PINOUT -----------------


//Ultima act

const int STEP1 = 41;  //MXLI STEP Yellow
const int DIR1 = 47;   //MXLI DIR Black
const int STEP2 = 42; //MXLE STEP Green
const int DIR2 = 21;  //MXLE Dir White
const int STEP3 = 2;// MXRI Step Blue
const int DIR3 = 20; // MXRI Dir Grey
const int STEP4 = 1; //MXRE Step Purple
const int DIR4 = 19; // MXRE Dir Purple
const int STEP5 = 12; // ZR Step Yellow
const int DIR5 = 11; // ZR Dir Orange
const int STEP6 = 10; // ZL Step Blue
const int DIR6 = 9; // ZL Dir Green

const int enable = 40; //Enable X
const int enable_z = 5; // Enable Z
 
const int YLE = 39; //36 vp END-STOP Y AXIS, LEFT EXTERNAL
const int YLI = 38; //39 vn END-STOP Y AXIS, LEFT INTERNAL
const int YRE = 37; // 34 END-STOP Y AXIS, RIGHT EXTERNAL
const int YRI = 36; //35 END-STOP Y AXIS, RIGHT INTERNAL
const int ZL = 35; // 25 END-STOP Z AXIS, LEFT
const int ZR = 45;// 26 END-STOP Z AXIS, LEFT




// ---------------- VARIABLES -----------------
const long HOMING_SPEED_FAST = 400;
const long HOMING_SPEED_SLOW = 1000;
const long MAX_X_HOMING_STEPS = 1500;
const long MAX_Z_HOMING_STEPS = 1200;
const long DIR_CHANGE_DELAY_US = 10000; // 10 ms
const int  BACKOFF_STEPS     = 30;
long SAFE_STEPS;
long Speed = 600;         // microseconds

bool YLEDone  = false;
bool YLIDone = false;
bool YREDone  = false;
bool YRIDone = false;
bool ZLDone = false;
bool ZRDone = false;
int adjusted = 0;
long CurrentStep1 = 0;    // vertical
long CurrentStep2 = 0;    // horizontal

long verticalSteps;
long horizontalSteps;


long Step;
long diff;
// ---- individual motor step counters ----
long Step_MXLE = 0;
long Step_MXLI = 0;
long Step_MXRE = 0;
long Step_MXRI = 0;

long Step_MZL = 0;
long Step_MZR = 0;


static float residual_vertical = 0.0;
static float residual_horizontal = 0.0;

long computeStepMove(float target_mm, long current_step, float steps_per_mm,
                     long min_steps, float &residual) 
{
    float target_steps_f = target_mm * steps_per_mm;
    float diff = target_steps_f - current_step;
    diff += residual;
    if (abs(diff) < min_steps) {
        residual = diff;
        return 0;
    }
    long steps_to_move = (long)diff;
    residual = diff - steps_to_move;
    return steps_to_move;
}

void move(float xmm, float zmm) {

    long steps_vertical = computeStepMove(xmm, CurrentStep1, 25.0, 10, residual_vertical);
    if (steps_vertical != 0) {
        diff = steps_vertical;
        Step = abs(diff);
        digitalWrite(enable, LOW);
        if (diff > 0){
            digitalWrite(DIR1, LOW);
            digitalWrite(DIR2, LOW);
            digitalWrite(DIR3, LOW);
            digitalWrite(DIR4, LOW);
        } else {
            digitalWrite(DIR1, HIGH);
            digitalWrite(DIR2, HIGH);
            digitalWrite(DIR3, HIGH);
            digitalWrite(DIR4, HIGH);
        }
        bool verticalDone = false;

        for (int i = 0; i < Step && !verticalDone; i++) {

            if (digitalRead(YRI) == HIGH || digitalRead(YRE) == HIGH ||
                digitalRead(YLI) == HIGH || digitalRead(YLE) == HIGH) {
                verticalDone = true;
                break;
            }

            digitalWrite(STEP1, LOW);
            digitalWrite(STEP2, LOW);
            digitalWrite(STEP3, LOW);
            digitalWrite(STEP4, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP1, HIGH);
            digitalWrite(STEP2, LOW);
            digitalWrite(STEP3, LOW);
            digitalWrite(STEP4, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP1, LOW);
            digitalWrite(STEP2, HIGH);
            digitalWrite(STEP3, LOW);
            digitalWrite(STEP4, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP1, LOW);
            digitalWrite(STEP2, LOW);
            digitalWrite(STEP3, HIGH);
            digitalWrite(STEP4, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP1, LOW);
            digitalWrite(STEP2, LOW);
            digitalWrite(STEP3, LOW);
            digitalWrite(STEP4, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);
        digitalWrite(DIR1, LOW);
        digitalWrite(DIR2, LOW);
        digitalWrite(DIR3, LOW);
        digitalWrite(DIR4, LOW);
        CurrentStep1 += steps_vertical;
        digitalWrite(enable, HIGH);
    }

    long steps_horizontal = computeStepMove(zmm, CurrentStep2, 20.0, 5, residual_horizontal);
    if (steps_horizontal != 0) {
        diff = steps_horizontal;
        Step = abs(diff);
        digitalWrite(enable_z, LOW);
        if(diff > 0) {
            digitalWrite(DIR5, LOW);
            digitalWrite(DIR6, HIGH);
        } else {
            digitalWrite(DIR5, HIGH);
            digitalWrite(DIR6, LOW);
        }
        for (int i = 0; i < Step; i++) {
            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP5, HIGH);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(Speed);
        }
        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);
        CurrentStep2 += steps_horizontal;
        digitalWrite(enable_z, HIGH);
    }
}


void GoHomePair(float& posX, float& posZ) {
   
    bool xHomingReached = false;
    bool zHomingReached = false;
   
    if (((digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH) &&
    (digitalRead(YLI) == HIGH && digitalRead(YLE) == HIGH)) && 
    (digitalRead(ZL) == HIGH && digitalRead(ZL) == HIGH)) return;
    
    Serial.println("[HOMING]");

    const long FAST = HOMING_SPEED_FAST;
    const long SLOW = HOMING_SPEED_SLOW;


    digitalWrite(enable, LOW);
    digitalWrite(enable_z, LOW);

    digitalWrite(DIR1, HIGH);
    digitalWrite(DIR2, HIGH);
    digitalWrite(DIR3, HIGH);
    digitalWrite(DIR4, HIGH);
    digitalWrite(DIR5, HIGH);
    digitalWrite(DIR6, LOW);


    // Homing fast

    // Vertical
    SAFE_STEPS = 0;
    while (!xHomingReached && SAFE_STEPS < MAX_X_HOMING_STEPS) {

        if ((digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH)||
        (digitalRead(YLI) == HIGH && digitalRead(YLE) == HIGH)) {
            xHomingReached = true;
            break;
        }

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(FAST);

        digitalWrite(STEP1, HIGH);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(FAST);

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, HIGH);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(FAST);

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, HIGH);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(FAST);

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, HIGH);

        delayMicroseconds(FAST);

        SAFE_STEPS++;
    }
    digitalWrite(STEP1, LOW);
    digitalWrite(STEP2, LOW);
    digitalWrite(STEP3, LOW);
    digitalWrite(STEP4, LOW);
    
    // Horizontal
    SAFE_STEPS = 0;
    while (!zHomingReached && SAFE_STEPS < MAX_Z_HOMING_STEPS) {
        if (digitalRead(ZL) == HIGH || digitalRead(ZR) == HIGH){
            zHomingReached = true;
            break;
        } 
        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);

        delayMicroseconds(FAST);
    
        digitalWrite(STEP5, HIGH);
        digitalWrite(STEP6, LOW);

        delayMicroseconds(FAST);

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, HIGH);
                 
        delayMicroseconds(FAST);
    }
    
    digitalWrite(STEP5, LOW);
    digitalWrite(STEP6, LOW);
    
    if (xHomingReached && zHomingReached) {
        BackoffAll(BACKOFF_STEPS, HOMING_SPEED_FAST);
        SecondTouchPair(HOMING_SPEED_SLOW);
    }

    digitalWrite(enable, HIGH);
    digitalWrite(enable_z, HIGH);
    CurrentStep1 = 0;
    CurrentStep2 = 0;
    posX = 0.0f;
    posZ = 0.0f;
}

void SecondTouchPair(long speed_us) {

    bool rVerticalDone = false;
    bool lVerticalDone = false;
    bool verticalDone = false;
    bool zLeftDone = false;
    bool zRightDone = false;

    digitalWrite(DIR1, HIGH);
    digitalWrite(DIR2, HIGH);
    digitalWrite(DIR3, HIGH);
    digitalWrite(DIR4, HIGH);
    digitalWrite(DIR5, HIGH);
    digitalWrite(DIR6, LOW);   
    

    // -------- VERTICAL SECOND TOUCH --------
    while (!verticalDone) {

        if (digitalRead(YLI) == HIGH && digitalRead(YLE) == HIGH &&
            digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH) {
            verticalDone = true;
            break;
        }

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);
        delayMicroseconds(speed_us);

        if (!lVerticalDone) {
            
            if (digitalRead(YLI) == HIGH && digitalRead(YLE) == HIGH) lVerticalDone = true;
           
            else{  
                digitalWrite(STEP1, HIGH);
                digitalWrite(STEP2, LOW);
                digitalWrite(STEP3, LOW);
                digitalWrite(STEP4, LOW);
                delayMicroseconds(speed_us);

                digitalWrite(STEP1, LOW);
                digitalWrite(STEP2, HIGH);
                digitalWrite(STEP3, LOW);
                digitalWrite(STEP4, LOW);
                delayMicroseconds(speed_us);
            }
          
        }

        if (!rVerticalDone) {
          
            if (digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH) rVerticalDone = true;
            
            else {
                digitalWrite(STEP1, LOW);
                digitalWrite(STEP2, LOW);
                digitalWrite(STEP3, HIGH);
                digitalWrite(STEP4, LOW);
                delayMicroseconds(speed_us);

                digitalWrite(STEP1, LOW);
                digitalWrite(STEP2, LOW);
                digitalWrite(STEP3, LOW);
                digitalWrite(STEP4, HIGH);
                delayMicroseconds(speed_us);
            }
        }
    }

    digitalWrite(STEP1, LOW);
    digitalWrite(STEP2, LOW);
    digitalWrite(STEP3, LOW);
    digitalWrite(STEP4, LOW);

    // -------- HORIZONTAL SECOND TOUCH --------
    while (!zLeftDone || !zRightDone) {

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);
        delayMicroseconds(speed_us);

        // RIGHT Z
        if (!zRightDone) {
            if (digitalRead(ZR) == HIGH) {
                zRightDone = true;
            } else {
                digitalWrite(STEP5, HIGH);
            }
        }

        // LEFT Z
        if (!zLeftDone) {
            if (digitalRead(ZL) == HIGH) {
                zLeftDone = true;
            } else {
                digitalWrite(STEP6, HIGH);
            }
        }

        delayMicroseconds(speed_us);
    }

    digitalWrite(STEP5, LOW);
    digitalWrite(STEP6, LOW);

    Serial.println("SecondTouch finished");
}



void BackoffAll(int steps, long speed_us) {

    digitalWrite(DIR1, LOW);
    digitalWrite(DIR2, LOW);
    digitalWrite(DIR3, LOW);
    digitalWrite(DIR4, LOW);
    digitalWrite(DIR5, LOW);
    digitalWrite(DIR6, HIGH);

    //Vertical
    for (int i = 0; i < steps; i++) {

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(speed_us);

        digitalWrite(STEP1, HIGH);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(speed_us);

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, HIGH);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(speed_us);

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, HIGH);
        digitalWrite(STEP4, LOW);

        delayMicroseconds(speed_us);

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, HIGH);

        delayMicroseconds(speed_us);  
    }
    
    digitalWrite(STEP1, LOW);
    digitalWrite(STEP2, LOW);
    digitalWrite(STEP3, LOW);
    digitalWrite(STEP4, LOW);
    
    // Horizontal
    for (int i = 0; i < steps; i++) {

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);

        delayMicroseconds(speed_us);
    
        digitalWrite(STEP5, HIGH);
        digitalWrite(STEP6, LOW);

        delayMicroseconds(speed_us);

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, HIGH);
                 
        delayMicroseconds(speed_us);
    }
    
    digitalWrite(STEP5, LOW);
    digitalWrite(STEP6, LOW);
}
void adjustmentZ() {
    const int STEPS_PER_MM = 20;

    const int Z_INIT_MM = 20;
    const int STEP_2MM = 2 * STEPS_PER_MM;   // 40
    const int STEP_4MM = 4 * STEPS_PER_MM;   // 80



    digitalWrite(enable_z, LOW);

    // --- Move to Z = +20 mm ---
    digitalWrite(DIR5, LOW);   // Z+
    digitalWrite(DIR6, HIGH);

    for (int i = 0; i < Z_INIT_MM * STEPS_PER_MM; i++) {
        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);
        delayMicroseconds(Speed);

        digitalWrite(STEP5, HIGH);
        digitalWrite(STEP6, LOW);
        delayMicroseconds(Speed);

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, HIGH);
        delayMicroseconds(Speed);
    }
    digitalWrite(STEP6, LOW);

    // --- Adjustment cycles ---
    for (int cycle = 0; cycle < 10; cycle++) {

        // --- Z- 2 mm ---
        digitalWrite(DIR5, HIGH);
        digitalWrite(DIR6, LOW);
        delayMicroseconds(DIR_CHANGE_DELAY_US);

        for (int i = 0; i < STEP_2MM; i++) {
            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, HIGH);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(Speed);
        }
        //if (digitalRead(YRE) == HIGH || digitalRead(YRI) == HIGH) adjusted = 1;
        // --- Z+ 4 mm ---
        digitalWrite(DIR5, LOW);
        digitalWrite(DIR6, HIGH);
        delayMicroseconds(DIR_CHANGE_DELAY_US);

        for (int i = 0; i < STEP_4MM; i++) {
            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, HIGH);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(Speed);
        }

        // --- Z- 4 mm ---
        digitalWrite(DIR5, HIGH);
        digitalWrite(DIR6, LOW);
        delayMicroseconds(DIR_CHANGE_DELAY_US);

        for (int i = 0; i < STEP_4MM; i++) {
            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, HIGH);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(Speed);
        }
        //if (digitalRead(YRE) == HIGH || digitalRead(YRI) == HIGH) adjusted = 1;

    }

    // --- Final correction: return to Z = 20 mm ---
    digitalWrite(DIR5, LOW);
    digitalWrite(DIR6, HIGH);
    delayMicroseconds(DIR_CHANGE_DELAY_US);

    for (int i = 0; i < STEP_2MM; i++) {
        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);
        delayMicroseconds(Speed);

        digitalWrite(STEP5, HIGH);
        digitalWrite(STEP6, LOW);
        delayMicroseconds(Speed);

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, HIGH);
        delayMicroseconds(Speed);
    }
    digitalWrite(STEP5, LOW);
    digitalWrite(STEP6, LOW);
    digitalWrite(enable_z, HIGH);

    Serial.println("Initial Z adjustment completed.");
}


void antiBacklashZ(int cycles, int steps, long speed_us) {

    digitalWrite(enable_z, LOW);

    for (int i = 0; i < cycles; i++) {

        // FORWARD
        delayMicroseconds(DIR_CHANGE_DELAY_US);  
        digitalWrite(DIR5, LOW);
        digitalWrite(DIR6, HIGH);

        for (int i = 0; i < steps; i++) {
            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(speed_us);

            digitalWrite(STEP5, HIGH);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(speed_us);

            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(speed_us);
        }

        // BACKWARD
        delayMicroseconds(DIR_CHANGE_DELAY_US);  
        digitalWrite(DIR5, HIGH);
        digitalWrite(DIR6, LOW);

        for (int i = 0; i < steps; i++) {
            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(speed_us);

            digitalWrite(STEP5, HIGH);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(speed_us);

            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(speed_us);
        }
    }

    digitalWrite(STEP5, LOW);
    digitalWrite(STEP6, LOW);
    digitalWrite(enable_z, HIGH);
}





// ---------------- SETUP -----------------

void init_motors() {

  pinMode(STEP1, OUTPUT);
	pinMode(DIR1, OUTPUT);
	pinMode(STEP2, OUTPUT);
	pinMode(DIR2, OUTPUT);
	pinMode(STEP3, OUTPUT);
	pinMode(DIR3, OUTPUT);
	pinMode(STEP4, OUTPUT);
	pinMode(DIR4, OUTPUT);
	pinMode(STEP5, OUTPUT);
	pinMode(DIR5, OUTPUT);
	pinMode(STEP6, OUTPUT);
	pinMode(DIR6, OUTPUT);

	pinMode(YLE, INPUT);
	pinMode(YLI, INPUT);
	pinMode(YRE, INPUT);
	pinMode(YRI, INPUT);
    pinMode(ZL, INPUT);
    pinMode(ZR, INPUT);

    pinMode(enable, OUTPUT);
    pinMode(enable_z, OUTPUT);
    digitalWrite(enable_z, HIGH);
    digitalWrite(enable, HIGH);

    digitalWrite(DIR1, HIGH);
    digitalWrite(DIR2, HIGH);
    digitalWrite(DIR3, HIGH);
    digitalWrite(DIR4, HIGH);
    digitalWrite(DIR5, HIGH);
    digitalWrite(DIR6, LOW);

    digitalWrite(STEP1, LOW);
    digitalWrite(STEP2, LOW);
    digitalWrite(STEP4, LOW);
    digitalWrite(STEP5, LOW);
    digitalWrite(STEP3, LOW);
    digitalWrite(STEP6, HIGH);
    adjustmentZ();
    antiBacklashZ(10,20, HOMING_SPEED_SLOW);
    Serial.println("Motors ready");
}






// --------------- INDIVIDUAL MOVEMENTS ------------

void move_external_vertical_right(float mm){
    verticalSteps = (long)(mm * 25);

    if (verticalSteps != Step_MXRE) {
        diff = verticalSteps - Step_MXRE;
        Step = abs(diff);
        digitalWrite(enable, LOW);

        digitalWrite(DIR4, diff > 0 ? LOW : HIGH);

        for (int i = 0; i < Step; i++) {
            digitalWrite(STEP4, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP4, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP4, LOW);
        digitalWrite(DIR4, LOW);

        Step_MXRE = verticalSteps;
    }
    digitalWrite(enable, HIGH);
}



void move_external_vertical_left(float mm){
    verticalSteps = (long)(mm * 25);

    if (verticalSteps != Step_MXLE) {
        diff = verticalSteps - Step_MXLE;
        Step = abs(diff);
        digitalWrite(enable, LOW);

        digitalWrite(DIR2, diff > 0 ? LOW : HIGH);

        for (int i = 0; i < Step; i++) {
            digitalWrite(STEP2, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP2, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP5, LOW);
        digitalWrite(DIR5, LOW);

        Step_MXLE = verticalSteps;
    }
    digitalWrite(enable, HIGH);
}



void move_internal_vertical_left(float mm){
    verticalSteps = (long)(mm * 25);

    if (verticalSteps != Step_MXLI) {
        diff = verticalSteps - Step_MXLI;
        Step = abs(diff);
        digitalWrite(enable, LOW);

        digitalWrite(DIR4, diff > 0 ? LOW : HIGH);

        for (int i = 0; i < Step; i++) {
            digitalWrite(STEP1, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP1, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP1, LOW);
        digitalWrite(DIR1, LOW);

        Step_MXLI = verticalSteps;
    }
    digitalWrite(enable, HIGH);
}
void move_internal_vertical_right(float mm){
    verticalSteps = (long)(mm * 25);

    if (verticalSteps != Step_MXRI) {
        diff = verticalSteps - Step_MXRI;
        Step = abs(diff);
        digitalWrite(enable, LOW);

        digitalWrite(DIR1, diff > 0 ? LOW : HIGH);

        for (int i = 0; i < Step; i++) {
            digitalWrite(STEP3, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP1, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP3, LOW);
        digitalWrite(DIR3, LOW);

        Step_MXRI = verticalSteps;
    }
    digitalWrite(enable, HIGH);
}

void move_horizontal_left(float mm){
    long target = (long)(mm * 20);
    long diff = target - Step_MZL;
    long steps = abs(diff);

    if (steps == 0) return;

    digitalWrite(enable_z, LOW);
    digitalWrite(DIR6, diff > 0 ? LOW : HIGH);

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP6, LOW);
        delayMicroseconds(Speed);
        digitalWrite(STEP6, HIGH);
        delayMicroseconds(Speed);
    }
    digitalWrite(STEP6, LOW);
    Step_MZL = target;
    digitalWrite(enable_z, HIGH);
}

void move_horizontal_right(float mm){
    long target = (long)(mm * 20);
    long diff = target - Step_MZR;
    long steps = abs(diff);

    if (steps == 0) return;

    digitalWrite(enable_z, LOW);
    digitalWrite(DIR5, diff > 0 ? HIGH : LOW);

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP5, LOW);
        delayMicroseconds(Speed);
        digitalWrite(STEP5, HIGH);
        delayMicroseconds(Speed);
    }
    digitalWrite(STEP5, LOW);
    Step_MZR = target;
    digitalWrite(enable_z, HIGH);
}



