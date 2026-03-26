/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/




// for solar smart-window use ESP32 Wrover Module
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <NetworkClient.h>
#include <WiFiAP.h>

AsyncWebServer server(80);
// Set these to your desired credentials.
const char* ssid = "SW-Prototype";
const char* password = "solar";

// REPLACE WITH YOUR NETWORK CREDENTIALS
//const char* ssid = "FASTWEB-E23F4D";
//const char* password = "GAJ1MWZ6PA";
//const char* ssid = "Gianni";
//const char* password = "connetti";

//const char* ssid = "Xiaomi";
//const char* password = "99mnbvcxz";

//const char* ssid = "eduroam";
//const char* password = "";


const char* PARAM_FLOAT = "inputFloat";

const char* PARAM_INPUT_1 = "vertical";
//const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "horizontal";
//const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "speed";
//const char* PARAM_INPUT_3 = "input3";

float ConvToInt;
char Array[9];  //serve per la conversione da stringa a float passando per array

long steps1; //new target  motor 1
long vertmm; //new target  motor 1 in mm
long Step; // calcolo numero di step che farà il motore

long steps2; //new target motor 2
long horizmm; //new targhet  motor 2 in mm



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



long  CurrentStep1 ;
long  CurrentStep2 ;
long  Speed=600 ;

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    vertical: <input type="text" name="vertical">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    horizontal: <input type="text" name="horizontal">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    speed: <input type="text" name="speed">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  pinMode(STEP1, OUTPUT);  //
  pinMode(DIR1, OUTPUT);
    pinMode(STEP2, OUTPUT);
  pinMode(DIR2, OUTPUT);
    pinMode(STEP3, OUTPUT);
  pinMode(DIR3, OUTPUT);
    pinMode(STEP4, OUTPUT);
  pinMode(DIR4, OUTPUT);
    pinMode(STEP5, OUTPUT);  //
  pinMode(DIR5, OUTPUT);
    pinMode(STEP6, OUTPUT);
  pinMode(DIR6, OUTPUT);
//  pinMode(enable2, OUTPUT);
  pinMode(YLE, INPUT);
  pinMode(YLI, INPUT);
  pinMode(YRE, INPUT);
  pinMode(YRI, INPUT);
  pinMode(ZL, INPUT);
  pinMode(ZR, INPUT);
  
  
    
  pinMode(enable, OUTPUT);
  digitalWrite(enable, HIGH);
  //digitalWrite(enable2, HIGH);
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
  /*
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
*/
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      
      
      
      inputMessage.toCharArray(Array, 9);  //  convert in to array
      
      vertmm = atol(Array); //new target for motor 1 and 2 vertical
      //steps1 = vertmm ; 
      steps1 = vertmm *25;
      Serial.print (steps1);
      Serial.println(" steps for vertical movement ");
      Serial.print (vertmm);
      Serial.println(" displacement in mm for vertical movement ");
      
      if (steps1 > CurrentStep1) {
        Step = steps1-CurrentStep1; //  calculate the number of steps to reach forward
        CurrentStep1=steps1;
        digitalWrite(enable, LOW);
//        digitalWrite(enable2, LOW);
        digitalWrite(DIR1, LOW);
        digitalWrite(DIR2, LOW);
        digitalWrite(DIR4, LOW);
        digitalWrite(DIR5, LOW);
        
        
        Serial.print("Spinning vertical motors down for ...");
        Serial.print (Step);
        Serial.println(" steps ");
  
        for (int i = 0; i < Step; i++)
          {
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
         
         digitalWrite(STEP2, HIGH);
         digitalWrite(STEP5, HIGH);
         delayMicroseconds(Speed);
         digitalWrite(STEP2, LOW);
         digitalWrite(STEP5, LOW);

         
         digitalWrite(DIR1, LOW);
         digitalWrite(DIR2, LOW);
         digitalWrite(DIR4, LOW);
         digitalWrite(DIR5, LOW);
         
       }
       
       else {
          // significa che steps < CurrentStep e deve andare SOPRA
          
          Step = CurrentStep1-steps1; //calculate the number of steps to go up
          CurrentStep1=steps1;
          
          if (Step < 0) {
            Step=0;
          }
          CurrentStep1=steps1;  
          digitalWrite(enable, LOW);
 //         digitalWrite(enable2, LOW);
          digitalWrite(DIR1, HIGH);
          digitalWrite(DIR2, HIGH);
          digitalWrite(DIR4, HIGH);
          digitalWrite(DIR5, HIGH);
          
          Serial.print("Spinning vertical motors UP for... ");
          Serial.print (Step);
          Serial.println(" steps ");

         

          for (int i = 0; i < Step; i++)
          {
            //while (digitalRead(YRI) == LOW) {
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
            //} //end while 
            //else {
            //  Serial.println(" end-stop vertical  Y Right Internal high ");
            //}
          }
         
          digitalWrite(STEP2, LOW);
          digitalWrite(STEP5, LOW);  

          digitalWrite(DIR1, LOW);
          digitalWrite(DIR2, LOW);
          digitalWrite(DIR4, LOW);
          digitalWrite(DIR5, LOW);
       
        }  
        digitalWrite(enable, HIGH); //disable all drivers
//        digitalWrite(enable2, HIGH);
        Serial.print("current vertical step..");
        Serial.println (CurrentStep1);
    }


            
          
            /*
            if (digitalRead(YRI) == LOW) {
              digitalWrite(STEP1, HIGH);
            }
            else {
              Serial.println(" end-stop vertical  Y Right Internal high ");
            }
            
            digitalWrite(STEP2, LOW);

            if (digitalRead(YLI) == LOW) {
              digitalWrite(STEP4, HIGH);
            }
            else {
              Serial.println(" end-stop vertical  Y Left Internal high ");
            }
            
            digitalWrite(STEP5, LOW);
            delayMicroseconds(Speed);
            
            digitalWrite(STEP1, LOW);
            if (digitalRead(YRE) == LOW) {
              digitalWrite(STEP2, HIGH);
            }
            else {
              Serial.println(" end-stop vertical  Y Right External high ");
            }
            
            digitalWrite(STEP4, LOW);

            if (digitalRead(YLE) == LOW) {
              digitalWrite(STEP5, HIGH);
            }
            else {
              Serial.println(" end-stop vertical  Y Left External high ");
            }
            delayMicroseconds(Speed);
            
          }
         if (digitalRead(YRE) == LOW) {
              digitalWrite(STEP2, HIGH);
            }
        //digitalWrite(STEP2, HIGH); //last step for motor 2 right
        else {
              Serial.println(" end-stop vertical  Y Right External high ");
            }
         if (digitalRead(YLE) == LOW) {
              digitalWrite(STEP5, HIGH);
            }
        //digitalWrite(STEP5, HIGH); //last step for motor 2 left
        else {
              Serial.println(" end-stop vertical  Y Left External high ");
            }

            */
            
        
        
        
        

    //----------Second Input  Horizontal-------------------
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      
      inputMessage.toCharArray(Array, 9);  // convert in to array
      horizmm = atol(Array); //new target for motor 3 horizzontal
      steps2 = horizmm * 20;
      //steps2 = horizmm;
      Serial.print (steps2);
      Serial.println(" steps for horizontal movement ");
      Serial.print (horizmm);
      Serial.println(" displacement in mm for horizontal movement ");
      
      
      if (steps2 > CurrentStep2) {
        Step = steps2-CurrentStep2; // calculate the number of steps to reach forward
        CurrentStep2=steps2;
        digitalWrite(enable, LOW);
//        digitalWrite(enable2, LOW);
        digitalWrite(DIR3, HIGH);
        digitalWrite(DIR6, LOW);
        
        Serial.print("Spinning horizontal motor Clockwise for ...");
        Serial.print (Step);
        Serial.println(" steps ");
  
        for (int i = 0; i < Step; i++)
          {
            digitalWrite(STEP3, HIGH);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(Speed);
            digitalWrite(STEP3, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);
            
           }
         digitalWrite(DIR3, LOW);
         digitalWrite(DIR6, LOW);
         
       }
       
       else  {
          // significa che steps < CurrentStep e deve andare indietro
          
          Step = CurrentStep2-steps2; // calculate the number of steps to go backwards
          CurrentStep2=steps2;
          
          if (Step < 0) {
            Step=0;
          }
          CurrentStep2=steps2;  
          digitalWrite(enable, LOW);
//          digitalWrite(enable2, LOW);
          digitalWrite(DIR3, LOW);
          digitalWrite(DIR6, HIGH);
          
          Serial.print("Spinning horizontal motor Anti-Clockwise for... ");
          Serial.print (Step);
          Serial.println(" steps ");
        
          for (int i = 0; i < Step; i++)
          {
            digitalWrite(STEP3, HIGH);
            digitalWrite(STEP6, HIGH);
            delayMicroseconds(Speed);
            digitalWrite(STEP3, LOW);
            digitalWrite(STEP6, LOW);
            delayMicroseconds(Speed);
            
          }
         digitalWrite(DIR3, LOW);
         digitalWrite(DIR6, LOW);          
        } 
      digitalWrite(enable, HIGH); //disable all drivers
//      digitalWrite(enable2, HIGH);
      Serial.print("current horizontal step..");
      Serial.println (CurrentStep2);  
      
    }
    /*
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_FLOAT)) {
      inputMessage = request->getParam(PARAM_FLOAT)->value();
     // writeFile(SPIFFS, "/inputFloat.txt", inputMessage.c_str());
    }
    */
    
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;

      inputMessage.toCharArray(Array, 9);  // converto nell'array
      Speed = atol(Array); //new speed
      Serial.println (Speed);
    }
    
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  
}
