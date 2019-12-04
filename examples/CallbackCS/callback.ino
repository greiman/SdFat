#include <Wire.h>																// I2C
#include <SPI.h>																// SPI
#include <pcf8574_esp.h>														// PCF8574/PCF8575
#include "SdFat.h"
#define I2C_EXP_A	0x20
#define SDA_PAD		5															// GPIO
#define SCL_PAD		4															// GPIO
#define TFT_CS		0															// I2C bit
#define LED			7															// I2C bit
#define SD_CS		2															// I2C bit
#define TP_CS		1															// I2C bit

using namespace sdfat;

PCF857x		pcf8575(I2C_EXP_A, &Wire, true);
SdFat		sd;
SdFile		sdfile;
SdFile		root;


void tft_sdcs(bool enabled)
{
	if (enabled) pcf8575.write(SD_CS, LOW);										// SD on
	else pcf8575.write(SD_CS, HIGH);
}

void setup()
{
	Serial.begin(115200);														// UART
	delay(50);
	Serial.println("\r\nStart SETUP");
	Wire.setClock((uint32_t)400000);											// I2C clock
	Wire.begin(SDA_PAD, SCL_PAD);												// Init I2C
	pcf8575.begin();															// PCF8575

	if (!sd.begin(tft_sdcs)) sd.initErrorHalt();

	if (!root.open("/")) sd.errorHalt("open root failed");

	while (sdfile.openNext(&root, O_RDONLY))
	{
		sdfile.printFileSize(&Serial);
		Serial.write(' ');
		sdfile.printModifyDateTime(&Serial);
		Serial.write(' ');
		sdfile.printName(&Serial);

		if (sdfile.isDir())	Serial.write('/');

		Serial.println();
		sdfile.close();
	}

	if (root.getError()) Serial.println("openNext failed");
	else Serial.println("Done!");
}

void loop() {}
