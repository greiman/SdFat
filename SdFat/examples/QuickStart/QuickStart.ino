// Quick hardware test.
//
#include <SdFat.h>
//
// Set DISABLE_CHIP_SELECT to disable a second SPI device.
// For example, with the Ethernet shield, set DISABLE_CHIP_SELECT
// to 10 to disable the Ethernet controller.
const int8_t DISABLE_CHIP_SELECT = -1;
//
// Test with reduced SPI speed for breadboards.
// Change spiSpeed to SPI_FULL_SPEED for better performance
// Use SPI_QUARTER_SPEED for even slower SPI bus speed
const uint8_t spiSpeed = SPI_HALF_SPEED;
//------------------------------------------------------------------------------
// Normally the SdFat class is used in applications in place
// of Sd2Card, SdVolume, and SdFile for root.
// These internal classes are used here to diagnose problems.
Sd2Card card;
SdVolume volume;
SdFile root;

// Serial streams
ArduinoOutStream cout(Serial);

// input buffer for line
char cinBuf[40];
ArduinoInStream cin(Serial, cinBuf, sizeof(cinBuf));

// SD card chip select
int chipSelect;

void cardOrSpeed() {
  cout << pstr("Try another SD card or reduce the SPI bus speed.\n");
  cout << pstr("Edit spiSpeed in this sketch to change it.\n");
}

void reformatMsg() {
  cout << pstr("Try reformatting the card.  For best results use\n");
  cout << pstr("the SdFormatter sketch in SdFat/examples or download\n");
  cout << pstr("and use SDFormatter from www.sdcard.org/downloads.\n");
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // Wait for Leonardo.
  delay(1000);        // Delay for Due.
  
  cout << pstr("\nSPI pins:\n");
  cout << pstr("MOSI: ") << int(MOSI) << endl;  
  cout << pstr("MISO: ") << int(MISO) << endl;
  cout << pstr("SCK:  ") << int(SCK) << endl;
  
  if (DISABLE_CHIP_SELECT < 0) {
    cout << pstr(
      "\nBe sure to edit DISABLE_CHIP_SELECT if you have\n"
      "a second SPI device.  For example, with the Ethernet\n"
      "shield, DISABLE_CHIP_SELECT should be set to 10\n"
      "to disable the Ethernet controller.\n");
  }
  cout << pstr(
    "\nSD chip select is the key hardware option.\n"
    "Common values are:\n"
    "Arduino Ethernet shield, pin 4\n"
    "Sparkfun SD shield, pin 8\n"
    "Adafruit SD shields and modules, pin 10\n");
}

bool firstTry = true;
void loop() {
  // read any existing Serial data
  while (Serial.read() >= 0) {}

  if (!firstTry) cout << pstr("\nRestarting\n");
  firstTry = false;

  cout << pstr("\nEnter the chip select pin number: ");
  cin.readline();
  if (cin >> chipSelect) {
    cout << chipSelect << endl;
  } else {
    cout << pstr("\nInvalid pin number\n");
    return;
  }
  if (DISABLE_CHIP_SELECT < 0) {
    cout << pstr(
      "\nAssuming the SD is the only SPI device.\n"
      "Edit DISABLE_CHIP_SELECT to disable another device.\n");
  } else {
    cout << pstr("\nDisabling SPI device on pin ");
    cout << int(DISABLE_CHIP_SELECT) << endl;
    pinMode(DISABLE_CHIP_SELECT, OUTPUT);
    digitalWrite(DISABLE_CHIP_SELECT, HIGH);
  }
  if (!card.init(spiSpeed, chipSelect)) {
    cout << pstr(
      "\nSD initialization failed.\n"
      "Do not reformat the card!\n"
      "Is the card correctly inserted?\n"
      "Is chipSelect set to the correct value?\n"
      "Does another SPI device need to be disabled?\n"
      "Is there a wiring/soldering problem?\n");
    cout << pstr("errorCode: ") << hex << showbase << int(card.errorCode());
    cout << pstr(", errorData: ") << int(card.errorData());
    cout << dec << noshowbase << endl;
    return;
  }
  cout << pstr("\nCard successfully initialized.\n");
  cout << endl;

  uint32_t size = card.cardSize();
  if (size == 0) {
    cout << pstr("Can't determine the card size.\n");
    cardOrSpeed();
    return;
  }
  uint32_t sizeMB = 0.000512 * size + 0.5;
  cout << pstr("Card size: ") << sizeMB;
  cout << pstr(" MB (MB = 1,000,000 bytes)\n");
  cout << endl;

  if (!volume.init(&card)) {
    if (card.errorCode()) {
      cout << pstr("Can't read the card.\n");
      cardOrSpeed();
    } else {
      cout << pstr("Can't find a valid FAT16/FAT32 partition.\n");
      reformatMsg();
    }
    return;
  }
  cout << pstr("Volume is FAT") << int(volume.fatType());
  cout << pstr(", Cluster size (bytes): ") << 512L * volume.blocksPerCluster();
  cout << endl << endl;

  root.close();
  if (!root.openRoot(&volume)) {
    cout << pstr("Can't open root directory.\n");
    reformatMsg();
    return;
  }
  cout << pstr("Files found (name date time size):\n");
  root.ls(LS_R | LS_DATE | LS_SIZE);

  if ((sizeMB > 1100 && volume.blocksPerCluster() < 64)
    || (sizeMB < 2200 && volume.fatType() == 32)) {
    cout << pstr("\nThis card should be reformatted for best performance.\n");
    cout << pstr("Use a cluster size of 32 KB for cards larger than 1 GB.\n");
    cout << pstr("Only cards larger than 2 GB should be formatted FAT32.\n");
    reformatMsg();
    return;
  }
  // read any existing Serial data
  while (Serial.read() >= 0) {}
  cout << pstr("\nSuccess!  Type any character to restart.\n");
  while (Serial.read() < 0) {}
}