#include <Arduino.h>
#include <SdFat.h>

// Define the CS pin here if not default/working
#define SD_CS_PIN SS

const char file_name[] = "test_sdfat.txt";
const char test_data[] = "This data should be written to the text file.\n"
                         "!\"#$%&\'()*+,-./"
                         "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                         "abcdefghijklmnopqrstuvwxyz{|}~";
SdFat sd;
File file;

void setup() {
  Serial.begin(9600);

  // Start SD card
  if (!sd.begin(SD_CS_PIN, SPI_FULL_SPEED)) {
    Serial.println("SD begin() failed (check wiring/CS pin/card seating)");
    while (true){
    }
  }

  // Delete file if it exists
  if (sd.exists(file_name)) {
    Serial.println("Test file already exists, deleting");
    sd.remove(file_name);
  }

  // Create file and open in read/write mode
  oflag_t oflags = (O_CREAT | O_RDWR);
  if (!file.open(file_name, oflags)) {
    Serial.println("Failed to create file");
    while (true){
    }
  }
  Serial.println("File created");

  // Write some data
  size_t bytes_written = file.write(test_data);
  if (bytes_written <= 0) {
    Serial.println("Failed to write to SD card");
    while (true){
    }
  }
  Serial.print("Wrote ");
  Serial.print(bytes_written);
  Serial.println(" bytes to the file");

  // Close and reopen file
  file.close();
  file.open(file_name, oflags);

  // Read some of the data back, an null terminate it to make it a valid str
  char read_data[65];
  size_t bytes_read = file.read(read_data, 64);
  read_data[bytes_read] = '\0';

  Serial.print("Read ");
  Serial.print(bytes_read);
  Serial.print(" bytes, which are as follows: \n\n");
  Serial.println(read_data);
}

// Empty loop as program is oneshot
void loop() {}