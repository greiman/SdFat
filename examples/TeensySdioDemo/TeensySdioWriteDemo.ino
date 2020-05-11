/*
 * 
 * Modified version of MTP Blinky that allows you to switch between MTP and SD writing with the same sketch.
 * By Jared Reabow May 2020, written for teensy 3.5/3.6
 * 
 * 
  */
#include "SdFat.h"
SdFatSdio sd;
SdFatSdioEX sdEx;
File file;
  #include <MTP.h>
  MTPStorage_SD storage;
  MTPD          mtpd(&storage);
  #define usbVisible true
  bool checkSDInstalled(){
    if (!sdEx.begin()) {
      sd.initErrorHalt("SdFatSdioEX begin() failed");
      return false;
    } else {
      return true;
    }
    // make sdEx the current volume.
    //sdEx.chvol();
  }
  void enableUSB(){
      mtpd.loop();
  }



volatile int  status = 0;
volatile bool sdfound = 0;
volatile int  count = 1;
int counter = 0;
String outputString = "";

void rtc_seconds_isr() {//activates each second
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("Program cycles: ");Serial.print(counter);Serial.print(" ");
    Serial.print(" ");Serial.println(outputString);
    counter = 0;
    digitalWrite(LED_BUILTIN, LOW);
}


void setup() {

///seconds timer interrupt
    RTC_IER |= 0x10;  // Enable seconds IRQ from RTC peripheral
    NVIC_ENABLE_IRQ(IRQ_RTC_SECOND); // Enable seconds IRS function in NVIC
/// end of second timer interrupt

//enable 5v output
  PORTE_PCR6 = PORT_PCR_MUX(1);
  GPIOE_PDDR |= (1<<6);
  GPIOE_PSOR = (1<<6); // turn on USB host power  
///end of enabled 5v output

  Serial.begin(2000000);
  pinMode(LED_BUILTIN, OUTPUT);
  if (checkSDInstalled()==true) {
    sdfound = true;
  } else {
    sdfound = false;
  }
  
  pinMode(23,HIGH);
  Serial3.begin(57600);
  

}


void loop() {
  counter++;
  viewSDonUSB();
 /* while(Serial3.available()) {
    Serial.println(Serial3.readString());
  }*/
}


void viewSDonUSB(){
  if(sdfound == true){ // if an SD card is found and working
    if(usbVisible == true){// if the software has enabled SD view on PC
      outputString = ("usb storage enabled, system will run slower");
      if(digitalRead(23) == HIGH){ // if the hardware pin for SD view is enabled
        enableUSB(); //448473 cycles no pin check//
        digitalWriteFast(LED_BUILTIN,HIGH);
      } else { 
        outputString = ("HARDWARE SD TO PC DISABLED"); digitalWriteFast(LED_BUILTIN,LOW);
        runTest();
      } //377873 no pincheck
    } else {  outputString = ("LIBRARY SD TO PC DISABLED"); }
  } else { outputString = ("NO SD CARD DETECTED"); } 
}


void runTest() {
  if(sdEx.exists("TeensyDemo.txt")){
    Serial.println("FILE FOUND");
    delay(1000);
  } else {
    Serial.println("FILE NOT FOUND");
    delay(1000);
  }
  if (!file.open("TeensyDemo.txt", O_WRITE | O_CREAT | O_AT_END)) {
        Serial.println("SD CARD FILE NOT OPENED");
  } else {
    Serial.println("SD CARD FILE OPENED");
  }
  if (!file.println("I LIKE TRAINS in teensyMTP demo")) {
        Serial.println("write failed");
  } else {
    Serial.println("write success");
    delay(1000);
  }
  file.close();
}
  
