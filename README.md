The Arduino SdFat library provides read/write access to FAT16/FAT32
file systems on SD/SDHC flash cards.

SdFat requires Arduino 1.6x or greater.

To use SdFat, clone the repository or unzip the ZIP file and place the SdFat
folder into the libraries sub-folder in your main sketch folder.

For more information see the Manual installation section of this guide:

http://arduino.cc/en/Guide/Libraries 

A number of configuration options can be set by editing SdFatConfig.h
\#define macros.  See the html documentation for details

Read changes.txt if you have used previous releases of this library.

Please read the html documentation for this library.  Start with
html/index.html and read the Main Page.  Next go to the Classes tab and
read the documentation for the classes SdFat, SdBaseFile, SdFile, File,
StdioStream, ifstream, ofstream, and others.
 
A new class, "File", has been added to provide compatibility with the Arduino
SD.h library. To use SdFat with programs written for SD.h replace

```
#include <SD.h>
```

with these two lines:

```
#include "SdFat.h"
SdFat SD;
```

Please continue by reading the html documentation.

Updated 19 Jul 2016
