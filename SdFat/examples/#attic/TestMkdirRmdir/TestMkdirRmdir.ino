/*
 * This sketch is a test of subdirectory and file creation.
 * It also tests allocation of clusters to directories.
 *
 * It will create two subdirectories and create enough files 
 * to force the allocation of a cluster to each directory.
 *
 * More than 3000 files may be created on a FAT32 volume.
 *
 * Note: Some cards may 'stutter' others just get slow due 
 * to the number of flash erases this program causes.
 */
#include <SdFat.h>
#include <SdFatUtil.h>

const uint8_t SD_CHIP_SELECT = SS;

SdFat sd;

// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))

/*
 * create enough files to force a cluster to be allocated to dir.
 */
void dirAllocTest(SdBaseFile* dir) {
  char buf[13], name[13];
  SdFile file;
  uint16_t n; 
  uint32_t size = dir->fileSize();
 
  // create files and write name to file
  for (n = 0; ; n++){
    // make file name
    sprintf(name, "%u.TXT", n);
   
    // open start time
    uint32_t t0 = millis();
    if (!file.open(dir, name, O_WRITE | O_CREAT | O_EXCL)) {
      error("open for write failed");
    }
   
    // open end time and write start time
    uint32_t t1 = millis();
    // write file name to file
    file.print(name);
    if (!file.close()) error("close write");
   
    // write end time
    uint32_t t2 = millis();
    PgmPrint("WR ");
    Serial.print(n);
    Serial.write(' ');
   
    // print time to create file
    Serial.print(t1 - t0);
    Serial.write(' ');
   
    // print time to write file
    Serial.println(t2 - t1);
   
    // directory size will change when a cluster is added
    if (dir->fileSize() != size) break;
  }

  // read files and check content
  for (uint16_t i = 0; i <= n; i++) {
    sprintf(name, "%u.TXT", i);
   
    // open start time
    uint32_t t0 = millis();
    if (!file.open(dir, name, O_READ)) {
      error("open for read failed");
    }
    
    // open end time and read start time
    uint32_t t1 = millis();
    int16_t nr = file.read(buf, 13);
    if (nr < 5) error("file.read failed");
    
    // read end time
    uint32_t t2 = millis();
    
    // check file content
    if (strlen(name) != nr || strncmp(name, buf, nr)) {
      error("content compare failed");
    }
    if (!file.close()) error("close read failed");
    
    PgmPrint("RD ");
    Serial.print(i);
    Serial.write(' ');
    
    // print open time
    Serial.print(t1 - t0);
    Serial.write(' ');
    
    // print read time
    Serial.println(t2 - t1);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo
  PgmPrintln("Type any character to start");
  while (Serial.read() <= 0) {}
  delay(200);  // Catch Due reset problem
  
  // initialize the SD card at SPI_FULL_SPEED for best performance.
  // try SPI_HALF_SPEED if bus errors occur.
  if (!sd.begin(SD_CHIP_SELECT, SPI_FULL_SPEED)) sd.initErrorHalt();
    
  // write files to root if FAT32
  if (sd.vol()->fatType() == 32) {
    PgmPrintln("Writing files to root");
    dirAllocTest(sd.vwd());
  }
  
  // create sub1 and write files
  SdFile sub1;
  if (!sub1.makeDir(sd.vwd(), "SUB1")) error("makdeDir SUB1 failed");
  PgmPrintln("Writing files to SUB1");
  dirAllocTest(&sub1);

  // create sub2 and write files
  SdFile sub2;
  if (!sub2.makeDir(&sub1, "SUB2")) error("makeDir SUB2 failed");
  PgmPrintln("Writing files to SUB2"); 
  dirAllocTest(&sub2);
  
  PgmPrintln("Done");
}

void loop() { }
