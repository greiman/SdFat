// Benchmark comparing SdFile and StdioStream.
#include <SdFat.h>

// Define PRINT_FIELD nonzero to use printField.
#define PRINT_FIELD 0

// Number of lines to list on Serial.
#define STDIO_LIST_COUNT 0
#define VERIFY_CONTENT 0

const uint8_t SD_CS_PIN = SS;
SdFat sd;

SdFile printFile;
StdioStream stdioFile;

float f[100];
char buf[20];
char* label[] = 
  {"uint8_t 0 to 255, 100 times ", "uint16_t 0 to 20000",
  "uint32_t 0 to 20000", "uint32_t 1000000000 to 1000010000",
  "float nnn.ffff, 10000 times"};
//------------------------------------------------------------------------------
void setup() {
  uint32_t m;
  uint32_t printSize;
  uint32_t stdioSize;
  uint32_t printTime;
  uint32_t stdioTime;
  
  Serial.begin(9600);
  Serial.println(F("Type any character to start"));
  while (!Serial.available());
  Serial.println(F("Starting test"));
  if (!sd.begin(SD_CS_PIN)) sd.errorHalt();

  for (uint8_t i = 0; i < 100; i++) {
    f[i] = 123.0 + 0.12345*i;
  }  

  for (uint8_t dataType = 0; dataType < 5; dataType++) {
    for (uint8_t fileType = 0; fileType < 2; fileType++) {
      if (!fileType) {
        if (!printFile.open("PRRINT.TXT", O_CREAT | O_RDWR | O_TRUNC)) {
          Serial.println("open fail");
            return;
        }
        printTime = millis();
        switch (dataType) {
        case 0:
          for (uint16_t i =0; i < 100; i++) {
            for (uint8_t j = 0; j < 255; j++) {
              printFile.println(j);
            }
          }            
          break;
        case 1:
          for (uint16_t i = 0; i < 20000; i++) {
            printFile.println(i);
          }
          break;
             
        case 2:
          for (uint32_t i = 0; i < 20000; i++) {
            printFile.println(i);
          }
          break;
             
        case 3:
          for (uint16_t i = 0; i < 10000; i++) {
            printFile.println(i + 1000000000UL);
          }
          break;
        
        case 4:
          for (int j = 0; j < 100; j++) {
            for (uint8_t i = 0; i < 100; i++) {
              printFile.println(f[i], 4);
            }
          }
          break;        
        default:
          break;
        }
        printFile.sync();        
        printTime = millis() - printTime;
        printFile.rewind();
        printSize = printFile.fileSize(); 

      } else {
        if (!stdioFile.fopen("STREAM.TXT", "w+")) {
          Serial.println("fopen fail");
          return;
        }
        stdioTime = millis();
        
         switch (dataType) {
        case 0:
          for (uint16_t i =0; i < 100; i++) {
            for (uint8_t j = 0; j < 255; j++) {
              #if PRINT_FIELD
              stdioFile.printField(j, '\n');
              #else  // PRINT_FIELD
              stdioFile.println(j);
              #endif  // PRINT_FIELD
            }
          }            
          break;
        case 1:
          for (uint16_t i = 0; i < 20000; i++) {
            #if PRINT_FIELD
            stdioFile.printField(i, '\n');
            #else  // PRINT_FIELD
            stdioFile.println(i);
            #endif  // PRINT_FIELD
          }
          break;
             
        case 2:
          for (uint32_t i = 0; i < 20000; i++) {
            #if PRINT_FIELD
            stdioFile.printField(i, '\n');
            #else  // PRINT_FIELD
            stdioFile.println(i);
            #endif  // PRINT_FIELD
          }
          break;
             
        case 3:
          for (uint16_t i = 0; i < 10000; i++) {
            #if PRINT_FIELD
            stdioFile.printField(i + 1000000000UL, '\n');
            #else  // PRINT_FIELD
            stdioFile.println(i + 1000000000UL);
            #endif  // PRINT_FIELD      
          }
          break;
        
        case 4:
          for (int j = 0; j < 100; j++) {
            for (uint8_t i = 0; i < 100; i++) {
              #if PRINT_FIELD
              stdioFile.printField(f[i], '\n', 4);
              #else  // PRINT_FIELD
              stdioFile.println(f[i], 4);              
              #endif  // PRINT_FIELD                            
            }
          }
          break;        
        default:
          break;
        }
        stdioFile.fflush();
        stdioTime = millis() - stdioTime;
        stdioSize = stdioFile.ftell();   
        if (STDIO_LIST_COUNT) {
          size_t len;
          stdioFile.rewind();
          for (int i = 0; i < STDIO_LIST_COUNT; i++) {
            stdioFile.fgets(buf, sizeof(buf), &len);
            Serial.print(len);Serial.print(',');
            Serial.print(buf);
          }
        }

      }
     
    }
    Serial.println(label[dataType]);    
    if (VERIFY_CONTENT && printSize == stdioSize) {
      printFile.rewind();
      stdioFile.rewind();
      for (uint32_t i = 0; i < stdioSize; i++) {
        if (printFile.read() != stdioFile.getc()) {
          Serial.print(F("Files differ at pos: "));
          Serial.println(i);
          return;
        }
      }
    }

    Serial.print("fileSize: ");
    if (printSize != stdioSize) {
      Serial.print(printSize);
      Serial.print(" != ");
    }
    Serial.println(stdioSize);    
    Serial.print("print millis: ");
    Serial.println(printTime);
    Serial.print("stdio millis: ");
    Serial.println(stdioTime);
    Serial.print("ratio: ");
    Serial.println((float)printTime/(float)stdioTime);
    Serial.println();
    printFile.close();     
    stdioFile.fclose();    
  }
  Serial.println("Done");
}
void loop() {}