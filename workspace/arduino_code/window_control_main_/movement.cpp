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


// const iniciales
// const int DIR1 = 13;  //INTERNAL VERTICAL DIR, RIGHT  
// const int STEP1 = 12; //INTERNAL VERTICAL STEP, RIGHT associate with YRI end-STOP
// const int DIR2 = 14;  //EXTERNAL VERTICAL DIR, RIGHT  
// const int STEP2 = 27;  //EXTERNAL VERTICAL STEP, RIGHT  associate with YRE end-STOP
// const int DIR3 = 33;  // HORIZONTAL DIR, RIGHT
// const int STEP3 = 32;  // HORIZONTAL DIR, RIGHT         
// const int enable = 23;  // ENABLE right
// //const int enable2 = 5;  // ENABLE left

// const int DIR4 = 17;  //INTERNAL  VERTICAL DIR, LEFT  orange
// const int STEP4 = 16; //INTERNAL VERTICAL STEP,  LEFT yellow   associate with YLI end-STOP
// const int DIR5 = 4;  //EXTERNAL VERTICAL DIR,  LEFT green
// const int STEP5 = 0;  //EXTERNAL VERTICAL STEP,  LEFT      associate with YLE end-STOP
// const int DIR6 = 5;  // HORIZONTAL DIR,  LEFT
// const int STEP6 = 15;  // HORIZONTAL STEP,  LEFT

// const int YLE = 36; //END-STOP Y AXIS, LEFT EXTERNAL
// const int YLI = 39; //END-STOP Y AXIS, LEFT INTERNAL
// const int YRE = 34; //END-STOP Y AXIS, RIGHT EXTERNAL
// const int YRI = 35; //END-STOP Y AXIS, RIGHT INTERNAL
// const int ZL = 25;  //END-STOP Z AXIS, LEFT
// const int ZR = 26;  //END-STOP Z AXIS, LEFT







// ---------------- VARIABLES -----------------
const long HOMING_SPEED_FAST = 500;
const long HOMING_SPEED_SLOW = 1000;
const long MAX_X_HOMING_STEPS = 1500;
const long MAX_Z_HOMING_STEPS = 1200;
const int  BACKOFF_STEPS     = 30;
long SAFE_STEPS;

bool YLEDone  = false;
bool YLIDone = false;
bool YREDone  = false;
bool YRIDone = false;
bool ZLDone = false;
bool ZRDone = false;
long CurrentStep1 = 0;    // vertical
long CurrentStep2 = 0;    // horizontal
long Speed = 600;         // microseconds

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



void move(float xmm, float zmm) {

	verticalSteps = (long)(xmm * 25);   // vertical motors 1,2,4,5
	horizontalSteps = (long)(zmm * 20); // horizontal motors 3,6


  // ------ vertical movement ------
	if (verticalSteps != CurrentStep1) {
		diff = verticalSteps - CurrentStep1;
		Step = abs(diff);
		digitalWrite(enable, LOW);
	
		if (diff > 0){
			digitalWrite(DIR1, LOW);
			digitalWrite(DIR2, LOW);
			digitalWrite(DIR3, LOW);
			digitalWrite(DIR4, LOW);
			Serial.print("Vertical down ");
		} 
		else {
			digitalWrite(DIR1, HIGH);
			digitalWrite(DIR2, HIGH);
			digitalWrite(DIR3, HIGH);
			digitalWrite(DIR4, HIGH);
			Serial.print("Vertical up ");
		}
		Serial.print(Step);
		Serial.println(" steps");
		
		for (int i = 0; i < Step; i++) {
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
            digitalWrite(STEP4, LOW);

		// Reset DIR
		digitalWrite(DIR1, LOW);
		digitalWrite(DIR2, LOW);
		digitalWrite(DIR3, LOW);
		digitalWrite(DIR4, LOW);
		CurrentStep1 = verticalSteps;
		Serial.println("Vertical movement finished.");
	}
	Serial.print("current vertical step..");
	Serial.println (CurrentStep1);



	// ------ Horizontal movement ------

	if (horizontalSteps != CurrentStep2) {
		diff = horizontalSteps - CurrentStep2;
		Step = abs(diff);
		digitalWrite(enable_z, LOW);
		int estad = digitalRead(enable_z);
		if (estad == LOW){
			Serial.print("Enable Low");
		}
		else Serial.print("Enable high");
		if(diff > 0) {
			digitalWrite(DIR5, LOW);
			digitalWrite(DIR6, HIGH);
			Serial.print("horizontal forward ");
		}
		else{
			digitalWrite(DIR5, HIGH);
			digitalWrite(DIR6, LOW);
			Serial.print("horizontal backwards ");
		}
		
		Serial.print(Step);
		Serial.println(" steps");	

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
		digitalWrite(STEP6, LOW);
		CurrentStep2 = horizontalSteps;

		Serial.println("Horizontal movement completed.");

		Serial.print("current vertical step..");
        Serial.println (CurrentStep1);
	}
	Serial.print("current horizontal step..");
    Serial.println (CurrentStep2);
    digitalWrite(enable, HIGH);
    digitalWrite(enable_z, HIGH);
}

void GoHomePair(float& posX, float& posZ) {
    if (
    (digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH) && digitalRead(ZL) == HIGH) return;

    Serial.println("[HOMING]");

    const long FAST = HOMING_SPEED_FAST;
    const long SLOW = HOMING_SPEED_SLOW;
    bool xHomingReached = false;
    bool zHomingReached = false;

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
        SAFE_STEPS++;
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

        if (digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH) xHomingReached = true;
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

        if (digitalRead(ZL) == HIGH) zHomingReached = true;
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

    digitalWrite(DIR1, HIGH);
    digitalWrite(DIR2, HIGH);
    digitalWrite(DIR3, HIGH);
    digitalWrite(DIR4, HIGH);
    digitalWrite(DIR5, HIGH);
    digitalWrite(DIR6, LOW);   

    // while (
    // !(digitalRead(YLI) == HIGH && digitalRead(YLE) == HIGH) ||
    // !(digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH)
    // ) {
    while (!(digitalRead(YRI) == HIGH && digitalRead(YRE) == HIGH)){
        
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

    while (digitalRead(ZL) != HIGH) {

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

    Serial.println("HOMING Finished");
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


void antiBacklashZ(int cycles, int steps, long speed_us) {

    digitalWrite(enable_z, LOW);

    for (int i = 0; i < cycles; i++) {

        // FORWARD
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
    antiBacklashZ(5, 40, Speed);
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



