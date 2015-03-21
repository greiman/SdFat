#include <SPI.h>
#include <SdFat.h>

//  create a serial output stream
ArduinoOutStream cout(Serial);

void setup() {
  Serial.begin(9600);

  while (!Serial) {}  // wait for Leonardo
  delay(2000);

  cout << "Hello, World!\n";
}

void loop() {}
