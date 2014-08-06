/*
 * Sketch to compare size of SdFat V2 with Arduino SD library.
 * See SD_Size.pde for Arduino SD sketch.
 *
 */
#include <SdFat.h>

SdFat sd;

SdFile file;
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo

  if (!sd.begin()) {
    Serial.println("begin failed");
    return;
  }
  file.open("SIZE_TST.TXT", O_RDWR | O_CREAT | O_AT_END);

  file.println("Hello");

  file.close();
  Serial.println("Done");
}
//------------------------------------------------------------------------------
void loop() {}
