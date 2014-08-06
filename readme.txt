Please check the changes.txt file!

For those who don't like too much documentation read QuickStart.txt.

The Arduino SdFat library provides read/write access to FAT16/FAT32
file systems on SD/SDHC flash cards.

SdFat requires Arduino 1.05 or greater.

To use SdFat, unzip the download file and place the SdFat folder
into the libraries subfolder in your main sketch folder.  You may
need to create the libraries folder.  Restart the Arduino IDE if
it was open.  See the Arduino site for more details om installing libraries.

The default chip select for the SD card is the hardware SS pin.  On a
168/328 Arduino this is pin 10 and on a Mega this is pin 53.  If you are
using another pin for chip select you will need call SdFat::begin(chipSelectPin)
or SdFat::begin(chipSelectPin, sckRateID) with second parameter set to the
SPI rate ID.

If you have a shield like the SparkFun shield that uses pin 8 for chip
select you would change the line:
  sd.begin();
to
  sd.begin(8);
or
  sd.begin(8, SPI_HALF_SPEED);
to use a slower SPI clock.
  
If the example uses
  sd.init();
change it to:
  sd.begin(8, SPI_FULL_SPEED);


A number of configuration options can be set by editing SdFatConfig.h
#define macros.  Options include:

USE_SD_CRC - enable or disable SD card crc checking.

USE_MULTIPLE_CARDS - enable or disable use of multiple SD card sockets.

USE_SERIAL_FOR_STD_OUT - use Serial for the default stdOut.

ENDL_CALLS_FLUSH - enable a flush() call after endl.

SPI_SCK_INIT_DIVISOR - set the SPI rate for card initialization.

LEONARDO_SOFT_SPI - use software SPI on Leonardo Arduinos.

MEGA_SOFT_SPI - use software SPI on Mega Arduinos.

USE_SOFTWARE_SPI - always use software SPI.


If you wish to report bugs or have comments, send email to
fat16lib@sbcglobal.net

Read changes.txt if you have used previous releases of this library.

Read troubleshooting.txt for common hardware problems.

Please read the html documentation for this library.  Start with
html/index.html and read the Main Page.  Next go to the Classes tab and
read the documentation for the classes SdFat, SdFile, ifstream, ofstream.

The SdFile class implements binary files similar to Linux's system calls.

The StdioStream class implements buffered I/O for similar to Linux stdio.h.
StdioStream is under development so expect changes and bugs.

The classes ifstream, ofstream, istream, and ostream follow the
C++ iostream standard when possible.

Many examples are included in the SdFat/examples folder.

There are many tutorials and much documentation about using C++ iostreams
on the web.

http://www.cplusplus.com/  is a good C++ site for learning iostreams.

Arduinos access SD cards using the cards SPI protocol.  PCs, Macs, and
most consumer devices use the 4-bit parallel SD protocol.  A card that
functions well on A PC or Mac may not work well on the Arduino.

Most cards have good SPI read performance but cards vary widely in SPI
write performance.  Write performance is limited by how efficiently the
card manages internal erase/remapping operations.  The Arduino cannot
optimize writes to reduce erase operations because of its limited RAM.

SanDisk cards generally have good write performance.  They seem to have
more internal RAM buffering than other cards and therefore can limit
the number of flash erase operations that the Arduino forces due to its
limited RAM.

The hardware interface to the SD card should not use a resistor based
level shifter.  SdFat sets the SPI bus frequency to 8 MHz which results
in signal rise times that are too slow for the edge detectors in many
newer SD card controllers when resistor voltage dividers are used.

The 5 to 3.3 V level shifter for 5 V arduinos should be IC based like
the 74HC4050N based circuit shown in the file SdLevel.png.  The
Adafruit Wave Shield uses a 74AHC125N.  Gravitech sells SD and MicroSD
Card Adapters based on the 74LCX245.

If you are using a resistor based level shifter and are having problems
try setting the SPI bus frequency to 4 MHz. This can be done by using
card.init(SPI_HALF_SPEED) to initialize the SD card.

A feature to use software SPI is available.  Software SPI is slower
than hardware SPI but allows any digital pins to be used.  See
SdFatConfig.h for software SPI definitions.

An many shields designed for an Uno can be use on an Arduino Mega
by defining MEGA_SOFT_SPI in SdFatConfig.h.


The best way to restore an SD card's format is to use SDFormatter
which can be downloaded from:

http://www.sdcard.org/consumers/formatter/

SDFormatter aligns flash erase boundaries with file
system structures which reduces write latency and file system overhead.

SDFormatter does not have an option for FAT type so it may format
small cards as FAT12.

The example sketch SdFatFormatter.ino will format smaller cards FAT16
so they can be used with SdFat.

The SdFatTestSuite folder contains development tests.

The SdFat/examples folder has the following sketches.  Older examples
and test programs are in the examples/#attic folder.

AnalogBinLogger - Fast AVR ADC logger - see the AnalogBinLoggerExtras folder.

bench - A read/write benchmark.

cin_cout - Demo of ArduinoInStream and ArduinoOutStream.

dataLogger - A simple modifiable data logger.

directoryFunctions - Demo of chdir(), ls(), mkdir(), and  rmdir().

fgets - Demo of the fgets read line/string function.

formating - Print a table with various formatting options.

getline - Example of getline from section 27.7.1.3 of the C++ standard.

LowLatencyLogger - A modifiable data logger for higher data rates.

OpenNext - Open all files in the root dir and print their filename.

PrintBenchmark - A simple benchmark for printing to a text file.

QuickStart - A sketch to quickly test your SD card and SD shield/module.

RawWrite - A test of raw write functions for contiguous files.

readCSV - Read a comma-separated value file using iostream extractors.

ReadWriteSdFat - SdFat version of Arduino SD ReadWrite example.

rename - A demo of SdFat::rename(old, new) and SdFile::rename(dirFile, newPath).

SdFormatter - This sketch will format an SD or SDHC card.

SdInfo - Initialize an SD card and analyze its structure for trouble shooting.

StressTest - Create and write files until the SD is full.

Timestamp - Sets file create, modify, and access timestamps.

TwoCards - Example using two SD cards.

To access these examples from the Arduino development environment
go to:  File -> Examples -> SdFat -> <Sketch Name>

Compile, upload to your Arduino and click on Serial Monitor to run
the example.


Updated 06 Aug 2014
