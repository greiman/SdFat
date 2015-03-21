/*
 * Program to test Short File Name character case flags.
 */
#include <SPI.h>
#include <SdFat.h>

SdFat sd;

SdFile file;
char* name[] = {
  "low.low", "low.Mix", "low.UP",
  "Mix.low", "Mix.Mix", "Mix.UP",
  "UP.low",  "UP.Mix",  "UP.UP"
};
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo
  Serial.println("type any character to start");
  while (Serial.read() < 0) {}
  if (!sd.begin()) {
    Serial.println("begin failed");
    return;
  }
  for (uint8_t i = 0; i < 9; i++) {
    sd.remove(name[i]);
    if (!file.open(name[i], O_RDWR | O_CREAT | O_EXCL)) {
      sd.errorHalt(name[i]);
    }
    file.println(name[i]);

    file.close();
  }
  sd.ls(LS_DATE|LS_SIZE);
  Serial.println("Done");
}
//------------------------------------------------------------------------------
void loop() {}
