#include <Arduino.h>
#include "movement.h"
#include "manual_mode.h"

// ---------------- PINOUT -----------------
//18 puede q sea otro naranja 
const int DIR1 = 13;   //INTERNAL VERTICAL DIR, RIGHT 
const int STEP1 = 12;  //INTERNAL VERTICAL STEP, RIGHT associate with YRI end-STOP
const int DIR2 = 15;    //14 EXTERNAL VERTICAL DIR, RIGHT
const int STEP2 = 7; // 27 EXTERNAL VERTICAL STEP, RIGHT  associate with YRE end-STOP
const int DIR3 = 5; // 33  HORIZONTAL DIR, RIGHT
const int STEP3 = 6;// 32  HORIZONTAL DIR, RIGHT
const int enable = 4; //23  ENABLE right
//1 y 2 eran 36 y 39
const int STEP4 = 8; //16 INTERNAL VERTICAL STEP,  LEFT yellow   associate with YLI end-STOP
const int DIR4 = 3; // 17 INTERNAL VERTICAL DIR, LEFT  orange
const int DIR5 = 9; //4  EXTERNAL VERTICAL DIR,  LEFT green
const int STEP5 = 46; //0  EXTERNAL VERTICAL STEP,  LEFT      associate with YLE end-STOP
const int DIR6 = 11; // 15  HORIZONTAL DIR,  LEFT
const int STEP6 = 10; //5  Z, left;  
//(quiza cambiar p5 por p15)
const int YLE = 35; //36 vp END-STOP Y AXIS, LEFT EXTERNAL
const int YLI = 36; //39 vn END-STOP Y AXIS, LEFT INTERNAL
const int YRE = 37; // 34 END-STOP Y AXIS, RIGHT EXTERNAL
const int YRI = 38; //35 END-STOP Y AXIS, RIGHT INTERNAL
const int ZL = 39; // 25 END-STOP Z AXIS, LEFT
const int ZR = 40;// 26 END-STOP Z AXIS, LEFT


// const int DIR1 = 13;  //INTERNAL VERTICAL DIR, RIGHT  
// const int STEP1 = 12; //INTERNAL VERTICAL STEP, RIGHT associate with YRI end-STOP
// const int DIR2 = 14;  //EXTERNAL VERTICAL DIR, RIGHT  
// const int STEP2 = 27;  //EXTERNAL VERTICAL STEP, RIGHT  associate with YRE end-STOP
// const int DIR3 = 33;  // HORIZONTAL DIR, RIGHT
// const int STEP3 = 32;  // HORIZONTAL DIR, RIGHT         
// const int enable = 23;  // ENABLE right
// //const int enable2 = 5;  // ENABLE left

// const int DIR4 = 17;  //INTERNAL VERTICAL DIR, LEFT  orange
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

long CurrentStep1 = 0;    // vertical
long CurrentStep2 = 0;    // horizontal
long Speed = 500;         // microseconds

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
  digitalWrite(enable, HIGH);

  digitalWrite(DIR1, LOW);
  digitalWrite(DIR2, LOW);
  digitalWrite(DIR3, LOW);
  digitalWrite(DIR4, LOW);
  digitalWrite(DIR5, LOW);
  digitalWrite(DIR6, LOW);

  digitalWrite(STEP1, LOW);
  digitalWrite(STEP2, LOW);
  digitalWrite(STEP4, LOW);
  digitalWrite(STEP5, LOW);
  digitalWrite(STEP3, LOW);
  digitalWrite(STEP6, LOW);

  Serial.println("Motors ready");
}


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
			digitalWrite(DIR4, LOW);
			digitalWrite(DIR5, LOW);
			Serial.print("Vertical down ");
		} 
		else {
			digitalWrite(DIR1, HIGH);
			digitalWrite(DIR2, HIGH);
			digitalWrite(DIR4, HIGH);
			digitalWrite(DIR5, HIGH);
			Serial.print("Vertical up ");
		}
		Serial.print(Step);
		Serial.println(" steps");
		
		for (int i = 0; i < Step; i++) {
		  digitalWrite(STEP1, HIGH);
		  digitalWrite(STEP2, LOW);
		  digitalWrite(STEP4, HIGH);
		  digitalWrite(STEP5, LOW);
		  delayMicroseconds(Speed);

		  digitalWrite(STEP1, LOW);
		  digitalWrite(STEP2, HIGH);
		  digitalWrite(STEP4, LOW);
		  digitalWrite(STEP5, HIGH);
		  delayMicroseconds(Speed);
		}
		digitalWrite(STEP2, LOW);
		digitalWrite(STEP5, LOW);

		// Reset DIR
		digitalWrite(DIR1, LOW);
		digitalWrite(DIR2, LOW);
		digitalWrite(DIR4, LOW);
		digitalWrite(DIR5, LOW);

		CurrentStep1 = verticalSteps;
		Serial.println("Vertical movement finished.");
	}
	Serial.print("current vertical step..");
	Serial.println (CurrentStep1);



	// ------ Horizontal movement ------

	if (horizontalSteps != CurrentStep2) {
		diff = horizontalSteps - CurrentStep2;
		Step = abs(diff);
		digitalWrite(enable, LOW);
		if(diff > 0) {
			digitalWrite(DIR3, HIGH);
			digitalWrite(DIR6, LOW);
			Serial.print("horizontal forward ");
		}
		else{
			digitalWrite(DIR3, LOW);
			digitalWrite(DIR6, HIGH);
			Serial.print("horizontal backwards ");
		}
		
		Serial.print(Step);
		Serial.println(" steps");	

		for (int i = 0; i < Step; i++) {
			digitalWrite(STEP3, HIGH);
			digitalWrite(STEP6, HIGH);
			delayMicroseconds(Speed);

			digitalWrite(STEP3, LOW);
			digitalWrite(STEP6, LOW);
			delayMicroseconds(Speed);
		}
		digitalWrite(STEP3, LOW);
		digitalWrite(STEP6, LOW);
		CurrentStep2 = horizontalSteps;

		Serial.println("Horizontal movement completed.");

		Serial.print("current vertical step..");
    Serial.println (CurrentStep1);
	}
	Serial.print("current horizontal step..");
  Serial.println (CurrentStep1);

	digitalWrite(enable, HIGH);
}




void move_external_vertical_right(float mm){
    verticalSteps = (long)(mm * 25);

    if (verticalSteps != Step_MXRE) {
        diff = verticalSteps - Step_MXRE;
        Step = abs(diff);
        digitalWrite(enable, LOW);

        digitalWrite(DIR2, diff > 0 ? LOW : HIGH);

        for (int i = 0; i < Step; i++) {
            digitalWrite(STEP2, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP2, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP2, LOW);
        digitalWrite(DIR2, LOW);

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

        digitalWrite(DIR5, diff > 0 ? LOW : HIGH);

        for (int i = 0; i < Step; i++) {
            digitalWrite(STEP5, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP5, HIGH);
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
            digitalWrite(STEP4, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP4, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP4, LOW);
        digitalWrite(DIR4, LOW);

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
            digitalWrite(STEP1, LOW);
            delayMicroseconds(Speed);
            digitalWrite(STEP1, HIGH);
            delayMicroseconds(Speed);
        }

        digitalWrite(STEP1, LOW);
        digitalWrite(DIR1, LOW);

        Step_MXRI = verticalSteps;
    }
    digitalWrite(enable, HIGH);
}

void move_horizontal_left(float mm){
    long target = (long)(mm * 20);
    long diff = target - Step_MZL;
    long steps = abs(diff);

    if (steps == 0) return;

    digitalWrite(enable, LOW);
    digitalWrite(DIR6, diff > 0 ? LOW : HIGH);

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP6, HIGH);
        delayMicroseconds(Speed);
        digitalWrite(STEP6, LOW);
        delayMicroseconds(Speed);
    }

    Step_MZL = target;
    digitalWrite(enable, HIGH);
}

void move_horizontal_right(float mm){
    long target = (long)(mm * 20);
    long diff = target - Step_MZR;
    long steps = abs(diff);

    if (steps == 0) return;

    digitalWrite(enable, LOW);
    digitalWrite(DIR3, diff > 0 ? LOW : HIGH);

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP3, HIGH);
        delayMicroseconds(Speed);
        digitalWrite(STEP3, LOW);
        delayMicroseconds(Speed);
    }

    Step_MZR = target;
    digitalWrite(enable, HIGH);
}



