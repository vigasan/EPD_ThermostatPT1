/**************************************************************************************************************************************************
* File name     : EPDTest.c
* Compiler      : 
* Autor         : VIGASAN   
* Created       : 20/04/2023
* Modified      : 
* Last modified :
*
*
* Description   : 
*
* Other info    : 
**************************************************************************************************************************************************/


/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------Include Files----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
#include <Adafruit_GFX.h>               // Core graphics library
#include "Adafruit_ThinkInk.h"
#include <SdFat.h>                      // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>          // SPI / QSPI flash library
#include <Adafruit_ImageReader_EPD.h>   // Image-reading functions

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------I/O Definitions--------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
#define EPD_DC              33 // can be any pin, but required!
#define EPD_CS              15 // can be any pin, but required!
#define SRAM_CS             -1 // can set to -1 to not use a pin (uses a lot of RAM!)
#define EPD_BUSY            -1 // can set to -1 to not use a pin (will wait a fixed delay)
#define EPD_RESET           -1 // can set to -1 and share with chip Reset (can't deep sleep)
#define SD_CS               14 // SD card chip select

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------Public variables-------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
ThinkInk_290_Tricolor_Z10   display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);  // E-Paper Display
SdFat                       SD;                                                     // SD card filesystem
Adafruit_ImageReader_EPD    reader(SD);                                             // Image-reader object, pass in SD filesys

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------ SETUP ----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
void setup(void) 
{
    Serial.begin(9600);                                         // Init Serial for Messages Monitor
    delay(500);

    display.begin();                                            // Init Display

    Serial.print("Initializing filesystem...");
    if(!SD.begin(SD_CS, SD_SCK_MHZ(10)))                        // Init SD Card File System
    {
        Serial.println("SD begin() failed");
        for(;;);                                                // Fatal error, do not continue
    }

    Serial.println("OK!");
    display.clearBuffer();
    display.display();                         // Pause 15 seconds before continuing because it's eInk
    delay(15 * 1000);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------ LOOP -----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
void loop() 
{
    ImageReturnCode stat;                                                   // Status from image-reading functions
    Serial.print("Loading Background BMP to canvas...");
    stat = reader.drawBMP((char *)"/subscribe.bmp", display, 88, 44);       // Reads bitmap fro display
    reader.printStatus(stat);                                               // How'd we do?
    display.display();                                                      // Show on Display
    delay(15 * 1000);
    display.clearBuffer();
    display.display(); 
    delay(15 * 1000);
}


