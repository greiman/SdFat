This is a major rewrite of core FAT code so bugs and compatibility
problems are likely.  

Please report problems to the email address listed in the 
"Bugs and Comments" section of the html documentation.

Here are the most recent changes:

Added support for Long File Names. See the LongFileName example.

Replaced the core SdFat code with FatLib, a generic FAT12/FAT16/FAT32
library.  This may result in bugs and backward compatibility problems.

Added SdFatSoftSpi, a software SPI template class.  See the SoftwareSpi
example.

Added SdFatLibSpi, a class that uses the Arduino SPI.h library.

Allow simultaneous use of hardware and software SPI with multiple cards.
See the ThreeCard example. 
 
Added the "File" class for compatibility with the Arduino SD.h library 

Added StreamParseInt example to demonstrate the SD.h API.

The Arduino SdFat library provides read/write access to FAT16/FAT32
file systems on SD/SDHC flash cards.

SdFat requires Arduino 1.05 or greater.

To use SdFat, clone the repository or unzip the ZIP file and place the SdFat
folder into the libraries sub-folder in your main sketch folder.

For more information see the Manual installation section of this guide:

http://arduino.cc/en/Guide/Libraries 

A number of configuration options can be set by editing SdFatConfig.h
#define macros.  See the html documentation for details

Read changes.txt if you have used previous releases of this library.

Please read the html documentation for this library.  Start with
html/index.html and read the Main Page.  Next go to the Classes tab and
read the documentation for the classes SdFat, SdBaseFile, SdFile, File,
StdioStream, ifstream, ofstream, and others.
 
Support has been added for Software SPI on AVR, Due, and Teensy 3.1 boards.

See the ThreeCard example for use of multiple SD cards with simultaneous 
use of hardware and software SPI.

A new class, "File", has been added to provide compatibility with the Arduino
SD.h library. To use SdFat with programs written for SD.h replace

#include <SD.h>

with these two lines:

#include <SdFat.h>
SdFat SD;

Please continue by reading the html documentation.

Updated 21 Mar 2015
