// raw_gps_echo.ino
HardwareSerial gpsSerial(2);
#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("RAW GPS ECHO: listening on UART2...");
}

void loop() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    Serial.write(c); // echo raw bytes to USB serial
  }
}
