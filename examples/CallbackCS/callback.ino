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

PCF857x		pcf8575(I2C_EXP_A, &Wire, true);
SdFat		sd;
File		sdfile;


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

	if (!sd.begin(tft_sdcs))
	{
		Serial.println("initialization failed!");
	}
	else
	{
	sdfile = sd.open("test.txt", FILE_WRITE);
	// if the file opened okay, write to it:
		if (sdfile)
		{
			Serial.print("Writing to test.txt...");
			sdfile.println("testing 1, 2, 3.");
			// close the file:
			sdfile.close();
			Serial.println("done.");
		}
		else
		{
			// if the file didn't open, print an error:
			Serial.println("error opening test.txt");
		}

		// re-open the file for reading:
		sdfile = sd.open("test.txt");

		if (sdfile)
		{
			Serial.println("test.txt:");

			// read from the file until there's nothing else in it:
			while (sdfile.available())
			{
				Serial.write(sdfile.read());
			}

			// close the file:
			sdfile.close();
		}
		else
		{
			// if the file didn't open, print an error:
			Serial.println("error opening test.txt");
		}
	}

}

void loop()
{
}
