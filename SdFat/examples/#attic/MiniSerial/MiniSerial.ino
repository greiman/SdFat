// This example illustrates use of SdFat's
// minimal unbuffered AVR Serial support.
//
// This is useful for debug and saves RAM
// Will not work on Due, Leonardo, or Teensy
#include <SdFat.h>
#include <SdFatUtil.h>
#ifndef UDR0
#error no AVR serial port0
#endif
void setup() {
  MiniSerial.begin(9600);
  MiniSerial.println(FreeRam());
  MiniSerial.println(F("Type any Character"));
  while(MiniSerial.read() < 0) {}
  MiniSerial.println(F("Done"));
}
void loop() {}
