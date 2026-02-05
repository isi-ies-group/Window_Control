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
static float acc_vertical = 0.0;
static float acc_horizontal = 0.0;
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

void move(float xmm, float zmm)
{
    // ---------- VERTICAL (X) ----------
    long targetStepsX = (long)(xmm * 25.0f);
    long diffX = targetStepsX - CurrentStep1;

    if (diffX != 0) {
        long steps = abs(diffX);

        digitalWrite(enable, LOW);

        if (diffX > 0) {
            // subir
            digitalWrite(DIR1, LOW);
            digitalWrite(DIR2, LOW);
            digitalWrite(DIR3, LOW);
            digitalWrite(DIR4, LOW);
        } else {
            // bajar
            digitalWrite(DIR1, HIGH);
            digitalWrite(DIR2, HIGH);
            digitalWrite(DIR3, HIGH);
            digitalWrite(DIR4, HIGH);
        }

        for (long i = 0; i < steps; i++) {

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

            if (i % 100 == 0) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        }

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);

        CurrentStep1 = targetStepsX;
        digitalWrite(enable, HIGH);
    }

    // ---------- HORIZONTAL (Z) ----------
    long targetStepsZ = (long)(zmm * 20.0f);
    long diffZ = targetStepsZ - CurrentStep2;

    if (diffZ != 0) {
        long steps = abs(diffZ);

        digitalWrite(enable_z, LOW);

        if (diffZ > 0) {
            digitalWrite(DIR5, LOW);
            digitalWrite(DIR6, HIGH);
        } else {
            digitalWrite(DIR5, HIGH);
            digitalWrite(DIR6, LOW);
        }

        for (long i = 0; i < steps; i++) {

            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, HIGH);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);

            digitalWrite(STEP5, LOW);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(Speed);

            if (i % 100 == 0) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        }

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);

        CurrentStep2 = targetStepsZ;
        digitalWrite(enable_z, HIGH);
    }
}



void GoHomePair(float& posX, float& posZ) {
   
    bool xHomingReached = false;
    bool zHomingReached = false;
   
	if (digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH &&
	digitalRead(YLI) == HIGH && digitalRead(YLE) == HIGH &&
	digitalRead(ZL) == HIGH && digitalRead(ZR) == HIGH) {
		return;
	}

    if (digitalRead(YRI) == HIGH || digitalRead(YRE) == HIGH) xHomingReached = true;
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

        if (digitalRead(YRI) == HIGH || digitalRead(YRE) == HIGH ||
        digitalRead(YLI) == HIGH || digitalRead(YLE) == HIGH) {
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

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);

        delayMicroseconds(FAST);
    
        digitalWrite(STEP5, HIGH);
        digitalWrite(STEP6, LOW);

        delayMicroseconds(FAST);

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, HIGH);
                 
        delayMicroseconds(FAST);

        if (digitalRead(ZL) == HIGH || digitalRead(ZR) == HIGH) zHomingReached = true;
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



void SecondTouchPair(long speed_us)
{
    bool verticalDone = false;
    bool zLeftDone = false;
    bool zRightDone = false;

    const long MAX_SECOND_TOUCH_STEPS = 50;
    long safeSteps = 0;

    // Direcciones hacia home
    digitalWrite(DIR1, HIGH);
    digitalWrite(DIR2, HIGH);
    digitalWrite(DIR3, HIGH);
    digitalWrite(DIR4, HIGH);
    digitalWrite(DIR5, HIGH);
    digitalWrite(DIR6, LOW);

    delayMicroseconds(DIR_CHANGE_DELAY_US);

    digitalWrite(enable, LOW);
    digitalWrite(enable_z, LOW);

    // -------- VERTICAL SECOND TOUCH --------
    while (!verticalDone && safeSteps < MAX_SECOND_TOUCH_STEPS) {

        if (digitalRead(YLI) == HIGH &&
            digitalRead(YLE) == HIGH &&
            digitalRead(YRI) == HIGH &&
            digitalRead(YRE) == HIGH) {
            verticalDone = true;
            break;
        }

        // Decidir ANTES de stepear
        bool moveMXLI = !digitalRead(YLI);
        bool moveMXLE = !digitalRead(YLE);
        bool moveMXRI = !digitalRead(YRI);
        bool moveMXRE = !digitalRead(YRE);

        digitalWrite(STEP1, LOW);
        digitalWrite(STEP2, LOW);
        digitalWrite(STEP3, LOW);
        digitalWrite(STEP4, LOW);
        delayMicroseconds(speed_us);

        if (moveMXLI) digitalWrite(STEP1, HIGH);
        if (moveMXLE) digitalWrite(STEP2, HIGH);
        if (moveMXRI) digitalWrite(STEP3, HIGH);
        if (moveMXRE) digitalWrite(STEP4, HIGH);

        delayMicroseconds(speed_us);

        safeSteps++;
    }

    digitalWrite(STEP1, LOW);
    digitalWrite(STEP2, LOW);
    digitalWrite(STEP3, LOW);
    digitalWrite(STEP4, LOW);

    // -------- HORIZONTAL SECOND TOUCH --------
    safeSteps = 0;

    while ((!zLeftDone || !zRightDone) && safeSteps < MAX_SECOND_TOUCH_STEPS) {

        // evaluar finales
        if (digitalRead(ZL) == HIGH) zLeftDone = true;
        if (digitalRead(ZR) == HIGH) zRightDone = true;

        bool moveZL = !zLeftDone;
        bool moveZR = !zRightDone;

        digitalWrite(STEP5, LOW);
        digitalWrite(STEP6, LOW);
        delayMicroseconds(speed_us);

        if (moveZR) digitalWrite(STEP5, HIGH); // RIGHT Z
        if (moveZL) digitalWrite(STEP6, HIGH); // LEFT Z

        delayMicroseconds(speed_us);

        safeSteps++;
    }
    digitalWrite(DIR5, LOW);
    digitalWrite(DIR6, HIGH);
    delayMicroseconds(speed_us);
    

    digitalWrite(STEP5, HIGH);
    digitalWrite(STEP6, HIGH);
    delayMicroseconds(speed_us);
    digitalWrite(STEP5, LOW);
    digitalWrite(STEP6, LOW);
    
    digitalWrite(DIR1, LOW);
    digitalWrite(DIR2, LOW);
    digitalWrite(DIR3, LOW);
    digitalWrite(DIR4, LOW);
    digitalWrite(DIR5, LOW);
    digitalWrite(DIR6, LOW);

    digitalWrite(enable, HIGH);
    digitalWrite(enable_z, HIGH);


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



