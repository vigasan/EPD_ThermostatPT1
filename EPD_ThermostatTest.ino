/**************************************************************************************************************************************************
* File name     : EPD_ThermostatTest.c
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
#include "DHT.h"

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------Local definitions------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

#define DHTTYPE   DHT22

//Bitmaps coordinates (top left)
#define X_BATTERY           4
#define Y_BATTERY           4

#define X_WIFI              2
#define Y_WIFI              106

#define X_TEMP_CURR_B       53
#define Y_TEMP_CURR_B_x1    38
#define Y_TEMP_CURR_B_x10   82
#define X_TEMP_CURR_DEC     84
#define Y_TEMP_CURR_DEC     6

#define X_TEMP_TARGET_B     127
#define Y_TEMP_TARGET_B_x1  41
#define Y_TEMP_TARGET_B_x10 77
#define X_TEMP_TARGET_DEC   155
#define Y_TEMP_TARGET_DEC   14

#define X_HUMIDITY_B        223
#define Y_HUMIDITY_B_x1     38
#define Y_HUMIDITY_B_x10    82
#define X_HUMIDITY_DEC      254
#define Y_HUMIDITY_DEC      6

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------I/O Definitions--------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
#define EPD_DC              33//10  // can be any pin, but required!
#define EPD_CS              15//9   // can be any pin, but required!
#define SRAM_CS             -1//6   // can set to -1 to not use a pin (uses a lot of RAM!)
#define EPD_BUSY            -1//7   // can set to -1 to not use a pin (will wait a fixed delay)
#define EPD_RESET           -1//8   // can set to -1 and share with chip Reset (can't deep sleep)
#define SD_CS               14//5   // SD card chip select

#define DHTPIN              22      // DHT22 data pin

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------Public variables-------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
ThinkInk_290_Tricolor_Z10   display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);  // E-Paper Display
SdFat                       SD;                                                     // SD card filesystem
Adafruit_ImageReader_EPD    reader(SD);                                             // Image-reader object, pass in SD filesys
DHT                         g_dht(DHTPIN, DHTTYPE);                                 // DHT22 Temperature Humidity Sensor
float                       g_Temperature = 1.0;
float                       g_Humidity = 1.0;
float                       g_TargetTemperature = 20.0;
int                         g_BatteyLevel = 80;
bool                        g_WifiConnected = true;


/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------ SETUP ----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
void setup(void) 
{
    g_dht.begin();                                              // Init DHT22 Temperature Humidity Sensor

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
    ImageReturnCode stat;                                       // Status from image-reading functions
    Serial.print("Loading Background BMP to canvas...");
    stat = reader.drawBMP((char *)"/bckg.bmp", display, 0, 0);  // Reads background bitmap 
    reader.printStatus(stat);                                   // How'd we do?
    display.display();                                          // Show on Display
    delay(15 * 1000);                                           // Pause 15 seconds before continuing because it's eInk
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------ LOOP -----------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
void loop() 
{
    ImageReturnCode stat;
    int x10 = 0, x1 = 0, dec = 0;                               // Integer parts of a float number (Ex.: 25.4 --> x10 = 2, x1 = 5, dec = 4)
    
    g_Temperature = g_dht.readTemperature();                    // Reads Temperature from DHT22 Sensor        
    g_Humidity = g_dht.readHumidity();                          // Reads Humidity from DHT22 Sensor 

    // RENDER BATTERY LEVEL
    stat = reader.drawBMP(GetBatteryBMP(g_BatteyLevel), display,  X_BATTERY, Y_BATTERY);

    // RENDER WIFI ICON
    if(g_WifiConnected)
        stat = reader.drawBMP((char *)"/wifi.bmp", display,  X_WIFI, Y_WIFI);
    else
        stat = reader.drawBMP((char *)"/nowifi.bmp", display,  X_WIFI, Y_WIFI);                         // Delete wifi bitmap (white BMP)

    // RENDER CURRENT TEMPERATURE
    ExtractDigits(g_Temperature, x10, x1, dec);   // Extraxts digits from float number (Ex.: 25.4 --> x10 = 2, x1 = 5, dec = 4)
    if(x10 == 0)
        stat = reader.drawBMP((char *)"/bbempty.bmp", display,  X_TEMP_CURR_B, Y_TEMP_CURR_B_x10);      // Delete digit if 0 (white BMP)
    else
        stat = reader.drawBMP(GetBigBlackDigitBMP(x10), display,  X_TEMP_CURR_B, Y_TEMP_CURR_B_x10);
    stat = reader.drawBMP(GetBigBlackDigitBMP(x1), display,  X_TEMP_CURR_B, Y_TEMP_CURR_B_x1); 
    stat = reader.drawBMP(GetLittleBlackDigitBMP(dec), display,  X_TEMP_CURR_DEC, Y_TEMP_CURR_DEC);

    // RENDER TARGET TEMPERATURE
    ExtractDigits(g_TargetTemperature, x10, x1, dec);                                                   // Extraxts digits from float number
    if(x10 == 0)
        stat = reader.drawBMP((char *)"/brempty.bmp", display,  X_TEMP_TARGET_B, Y_TEMP_TARGET_B_x10);  // Delete digit if 0 (white BMP)
    else
        stat = reader.drawBMP(GetBigRedDigitBMP(x10), display,  X_TEMP_TARGET_B, Y_TEMP_TARGET_B_x10);
    stat = reader.drawBMP(GetBigRedDigitBMP(x1), display,  X_TEMP_TARGET_B, Y_TEMP_TARGET_B_x1); 
    stat = reader.drawBMP(GetLittleRedDigitBMP(dec), display,  X_TEMP_TARGET_DEC, Y_TEMP_TARGET_DEC);

    // RENDER HUMIDITY
    ExtractDigits(g_Humidity, x10, x1, dec);                                                            // Extraxts digits from float number
    if(x10 == 0)
        stat = reader.drawBMP((char *)"/bbempty.bmp", display,  X_HUMIDITY_B, Y_HUMIDITY_B_x10);        // Delete digit if 0 (white BMP)
    else
        stat = reader.drawBMP(GetBigBlackDigitBMP(x10), display,  X_HUMIDITY_B, Y_HUMIDITY_B_x10);
    stat = reader.drawBMP(GetBigBlackDigitBMP(x1), display,  X_HUMIDITY_B, Y_HUMIDITY_B_x1); 
    stat = reader.drawBMP(GetLittleBlackDigitBMP(dec), display,  X_HUMIDITY_DEC, Y_HUMIDITY_DEC);

    // DISPLAY ALL DATA
    reader.printStatus(stat);
    display.display(); 

    delay(15 * 1000); // Pause 15 sec.
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------ Public Functions -----------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
void ExtractDigits(float val, int &x10, int &x1, int &dec)
{
    int intpart;
    float decpart;
    intpart = int(val);
    decpart = val - intpart;

    x10 = intpart / 10;
    x1 = intpart % 10;
    dec = int(decpart * 10);
}

char* GetBatteryBMP(int batteryLevel)
{
    char* bmpBattery = NULL;
    int battIMG = batteryLevel / 20;

    switch(battIMG)
    {
        case 0: bmpBattery = (char *)"/batt0.bmp"; break;
        case 1: bmpBattery = (char *)"/batt20.bmp"; break;
        case 2: bmpBattery = (char *)"/batt40.bmp"; break;
        case 3: bmpBattery = (char *)"/batt60.bmp"; break;
        case 4: bmpBattery = (char *)"/batt80.bmp"; break;
        case 5: bmpBattery = (char *)"/batt100.bmp"; break;
    }
    return bmpBattery;
}

char* GetBigBlackDigitBMP(int digit)
{
    char* bmpFileDigit = NULL;
    switch(digit)
    {
        case 0: bmpFileDigit = (char *)"/bb0.bmp"; break;
        case 1: bmpFileDigit = (char *)"/bb1.bmp"; break;
        case 2: bmpFileDigit = (char *)"/bb2.bmp"; break;
        case 3: bmpFileDigit = (char *)"/bb3.bmp"; break;
        case 4: bmpFileDigit = (char *)"/bb4.bmp"; break;
        case 5: bmpFileDigit = (char *)"/bb5.bmp"; break;
        case 6: bmpFileDigit = (char *)"/bb6.bmp"; break;
        case 7: bmpFileDigit = (char *)"/bb7.bmp"; break;
        case 8: bmpFileDigit = (char *)"/bb8.bmp"; break;
        case 9: bmpFileDigit = (char *)"/bb9.bmp"; break;

    }
    return bmpFileDigit;
}

char* GetLittleBlackDigitBMP(int digit)
{
    char* bmpFileDigit = NULL;
    switch(digit)
    {
        case 0: bmpFileDigit = (char *)"/lb0.bmp"; break;
        case 1: bmpFileDigit = (char *)"/lb1.bmp"; break;
        case 2: bmpFileDigit = (char *)"/lb2.bmp"; break;
        case 3: bmpFileDigit = (char *)"/lb3.bmp"; break;
        case 4: bmpFileDigit = (char *)"/lb4.bmp"; break;
        case 5: bmpFileDigit = (char *)"/lb5.bmp"; break;
        case 6: bmpFileDigit = (char *)"/lb6.bmp"; break;
        case 7: bmpFileDigit = (char *)"/lb7.bmp"; break;
        case 8: bmpFileDigit = (char *)"/lb8.bmp"; break;
        case 9: bmpFileDigit = (char *)"/lb9.bmp"; break;

    }
    return bmpFileDigit;
}

char* GetBigRedDigitBMP(int digit)
{
    char* bmpFileDigit = NULL;
    switch(digit)
    {
        case 0: bmpFileDigit = (char *)"/br0.bmp"; break;
        case 1: bmpFileDigit = (char *)"/br1.bmp"; break;
        case 2: bmpFileDigit = (char *)"/br2.bmp"; break;
        case 3: bmpFileDigit = (char *)"/br3.bmp"; break;
        case 4: bmpFileDigit = (char *)"/br4.bmp"; break;
        case 5: bmpFileDigit = (char *)"/br5.bmp"; break;
        case 6: bmpFileDigit = (char *)"/br6.bmp"; break;
        case 7: bmpFileDigit = (char *)"/br7.bmp"; break;
        case 8: bmpFileDigit = (char *)"/br8.bmp"; break;
        case 9: bmpFileDigit = (char *)"/br9.bmp"; break;

    }
    return bmpFileDigit;
}

char* GetLittleRedDigitBMP(int digit)
{
    char* bmpFileDigit = NULL;
    switch(digit)
    {
        case 0: bmpFileDigit = (char *)"/lr0.bmp"; break;
        case 1: bmpFileDigit = (char *)"/lr1.bmp"; break;
        case 2: bmpFileDigit = (char *)"/lr2.bmp"; break;
        case 3: bmpFileDigit = (char *)"/lr3.bmp"; break;
        case 4: bmpFileDigit = (char *)"/lr4.bmp"; break;
        case 5: bmpFileDigit = (char *)"/lr5.bmp"; break;
        case 6: bmpFileDigit = (char *)"/lr6.bmp"; break;
        case 7: bmpFileDigit = (char *)"/lr7.bmp"; break;
        case 8: bmpFileDigit = (char *)"/lr8.bmp"; break;
        case 9: bmpFileDigit = (char *)"/lr9.bmp"; break;

    }
    return bmpFileDigit;
}

