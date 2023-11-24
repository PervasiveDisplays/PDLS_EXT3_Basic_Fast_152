///
/// @file Common_Shapes.ino
/// @brief Example for Pervasive Displays Library Suite
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
void displayForms()
{
    myScreen.setOrientation(myOrientation);
    uint16_t x, y, dx, dy;
    uint16_t z, dz;

    x = myScreen.screenSizeX();
    y = myScreen.screenSizeY();
    dx = x / 6;
    dy = y / 6;
    z = min(x, y);
    dz = min(dx, dy);

    myScreen.setPenSolid(false);
    myScreen.dRectangle(0, 0, x, y, myColours.black);
    myScreen.dLine(0, 0, x, y, myColours.red);
    myScreen.triangle(dx * 3, dy * 2, dx * 5, dy, dx * 4, dy * 4,
                      myColours.black);

    myScreen.setPenSolid(true);
    myScreen.circle(dx * 2, dy * 2, dz, myColours.grey);
    myScreen.dRectangle(dx, dy * 4, dx * 2, dy, myColours.red);
    myScreen.triangle(dx * 4, dy * 5, dx * 5, dy * 3, dx * 5, dy * 5,
                      myColours.black);

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

    Serial.println("Forms... ");
    myScreen.clear();
    displayForms();
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
