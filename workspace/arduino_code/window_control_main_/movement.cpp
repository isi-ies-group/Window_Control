#include <Arduino.h>
#include "movement.h"

// ---------------- PINOUT -----------------

const int DIR1 = 13;  
const int STEP1 = 12; 
const int DIR2 = 14;  
const int STEP2 = 27;  
const int DIR3 = 33; 
const int STEP3 = 32;        
const int enable = 23;  

const int DIR4 = 6; //eran 17 y 16 para el italiano
const int STEP4 = 7; 
const int DIR5 = 4;  
const int STEP5 = 0;  
const int DIR6 = 5;  
const int STEP6 = 15;  

const int YLE = 36;
const int YLI = 39;
const int YRE = 34;
const int YRI = 35;
const int ZL = 25;
const int ZR = 26;

// ---------------- VARIABLES -----------------

long CurrentStep1 = 0;    // vertical
long CurrentStep2 = 0;    // horizontal
long Speed = 600;         // microseconds

long verticalSteps;
long horizontalSteps;
long Step;
long diff;
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
