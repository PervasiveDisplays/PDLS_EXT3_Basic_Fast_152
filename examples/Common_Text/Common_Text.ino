///
/// @file Common_Text.ino
/// @brief Example of features for basic edition
///
/// @details Project Pervasive Displays Library Suite
/// @n Based on highView technology
///
/// @author Rei Vilo
/// @date 21 Nov 2023
/// @version 702
///
/// @copyright (c) Rei Vilo, 2010-2023
/// @copyright Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
///
/// @see ReadMe.txt for references
/// @n
///

// Screen
#include "PDLS_EXT3_Basic_Fast_152.h"

// SDK
// #include <Arduino.h>
#include "hV_HAL_Peripherals.h"

// Include application, user and local libraries
// #include <SPI.h>

// Configuration
#include "hV_Configuration.h"

// Set parameters

// Define structures and classes

// Define variables and constants
Screen_EPD_EXT3_Fast myScreen(eScreen_EPD_EXT3_152_0J_Fast, boardRaspberryPiPico_RP2040);

// Prototypes

// Utilities
///
/// @brief Wait with countdown
/// @param second duration, s
///
void wait(uint8_t second)
{
    for (uint8_t i = second; i > 0; i--)
    {
        Serial.print(formatString(" > %i  \r", i));
        delay(1000);
    }
    Serial.print("         \r");
}

// Functions
///
/// @brief Characters test screen
/// @param flag true = default = perform flush, otherwise no
///
/// @image html T2_CHARA.jpg
/// @image latex T2_CHARA.PDF width=10cm
///
void displayCharacters(bool flag = true)
{
    myScreen.setOrientation(7);
    uint16_t x = myScreen.screenSizeX(); // 17
    uint16_t y = myScreen.screenSizeY(); // 14

    // #if (USE_FONT_MODE == USE_FONT_TERMINAL)
    myScreen.selectFont(Font_Terminal8x12);
    // #elif (USE_FONT_MODE == USE_FONT_HEADER)
    //
    // #elif (USE_FONT_MODE == USE_FONT_FLASH)
    //
    // #else
    // #error USE_FONT_MODE not defined
    // #endif

    uint8_t k;
    String text;
    uint16_t dx;

    for (uint8_t i = 1; i < 17; i++)
    {
        myScreen.gText(i * x / 17, 0, formatString(".%x", (i - 1)), myColours.red);
    }
    for (uint8_t j = 2; j < 16; j++)
    {
        myScreen.gText(0, (j - 1)*y / 15, formatString("%x.", (j)), myColours.red);
    }

    for (uint16_t i = 1; i < 17; i++)
    {
        for (uint8_t j = 2; j < 16; j++)
        {
            k = (i - 1) + j * 16;

            text = (String)char(k);
            dx = i * x / 17 + (x / 17 - myScreen.stringSizeX(text)) / 2;
            myScreen.gText(dx, (j - 1)*y / 15, text, myColours.black);
        }
    }

    myScreen.flush();
}

// Add setup code
///
/// @brief Setup
///
void setup()
{
    // Start
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("=== " __FILE__);
    Serial.println("=== " __DATE__ " " __TIME__);
    Serial.println();

    Serial.println("begin... ");
    myScreen.begin();
    Serial.println(formatString("%s %ix%i", myScreen.WhoAmI().c_str(), myScreen.screenSizeX(), myScreen.screenSizeY()));

    Serial.println("Characters... ");
    myScreen.clear();
    displayCharacters();
    wait(8);

    Serial.println("White... ");
    myScreen.clear();
    myScreen.flush();

    Serial.println("=== ");
    Serial.println();
}

// Add loop code
///
/// @brief Loop, empty
///
void loop()
{
    delay(1000);
}
