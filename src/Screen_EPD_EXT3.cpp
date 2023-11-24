//
// Screen_EPD_EXT3.cpp
// Library C++ code
// ----------------------------------
//
// Project Pervasive Displays Library Suite
// Based on highView technology
//
// Created by Rei Vilo, 28 Jun 2016
//
// Copyright (c) Rei Vilo, 2010-2023
// Licence Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
//
// Release 508: Added eScreen_EPD_EXT3_437_0C xE2437CS0Cx
// Release 509: Added eScreen_EPD_EXT3_271_Fast xE2271PS09x
// Release 510: Improved consistency with PDLS_EXT3_Advanced_Fast_Large
// Release 510: Improved partial update for eScreen_EPD_EXT3_370_0C xE2370CS0Cx
// Release 511: Improved stability for external SPI SRAM
// Release 512: Added support for 266 Armor
// Release 521: Added friend class File_All
// Release 529: Added support for pixmap file
// Release 530: Added support for external SRAM
// Release 531: Added getBoardPins
// Release 540: Added global update option
// Release 542: Simulated global update
// Release 543: Improved support for eScreen_EPD_EXT3_266_0C xE2266CS0Cx
// Release 601: Added support for screens with embedded fast update
// Release 602: Improve functions structure
// Release 603: Improved numbering consistency
// Release 603: Simplified partial update
// Release 604: Improved optional external power management
// Release 604: Improved speed for screens with embedded fast update
// Release 607: Refactored peripherals
// Release 607: Improved screens names consistency
// Release 608: Added screen report
// Release 608: Added support for screens with wide temperature
// Release 608: Added support for fast update xE2417PS0Dx xE2417KS0Dx
// Release 609: Added temperature management
// Release 609: Deprecated partial update
// Release 610: Removed partial update
// Release 611: Added support for eScreen_EPD_EXT3_154_0C_Wide xE2154KS0Cx
// Release 611: Added support for eScreen_EPD_EXT3_150_0J_Wide xE2150KS0Jx
// Release 613: Added x- and y-axis margins
// Release 700: Refactored screen and board functions
// Release 701: Improved calls of board functions
// Release 702: Added support for eScreen_EPD_EXT3_152_0J_Wide xE2152KS0Jx
//

// Library header
#include "Screen_EPD_EXT3.h"

#if defined(ENERGIA)
///
/// @brief Proxy for SPISettings
/// @details Not implemented in Energia
/// @see https://www.arduino.cc/en/Reference/SPISettings
///
struct _SPISettings_s
{
    uint32_t clock; ///< in Hz, checked against SPI_CLOCK_MAX = 16000000
    uint8_t bitOrder; ///< LSBFIRST, MSBFIRST
    uint8_t dataMode; ///< SPI_MODE0, SPI_MODE1, SPI_MODE2, SPI_MODE3
};
///
/// @brief SPI settings for screen
///
_SPISettings_s _settingScreen;
#else
///
/// @brief SPI settings for screen
///
SPISettings _settingScreen;
#endif // ENERGIA

#ifndef SPI_CLOCK_MAX
#define SPI_CLOCK_MAX 16000000
#endif

//
// === COG section
//
/// @cond

// Common settings

void Screen_EPD_EXT3_Fast::COG_initial(uint8_t updateMode)
{
    // Soft reset
    b_sendCommand8(0x12);
    digitalWrite(b_pin.panelDC, LOW); // Select
    b_waitBusy(LOW); // 150 and 152 specific

    // Work settings
    b_sendCommandData8(0x1a, u_temperature);

    if (updateMode == UPDATE_GLOBAL)
    {
        b_sendCommandData8(0x22, 0xd7);
    }
    else if (updateMode == UPDATE_FAST)
    {
        b_sendCommandData8(0x3c, 0xc0);
        b_sendCommandData8(0x22, 0xdf);
    }
}

void Screen_EPD_EXT3_Fast::COG_getUserData()
{
    // Empty
}

void Screen_EPD_EXT3_Fast::COG_sendImageDataFast()
{
    uint8_t * nextBuffer = u_newImage;
    uint8_t * previousBuffer = u_newImage + u_pageColourSize;

    b_sendIndexData(0x24, previousBuffer, u_frameSize); // Previous frame
    b_sendIndexData(0x26, nextBuffer, u_frameSize); // Next frame
    memcpy(previousBuffer, nextBuffer, u_frameSize); // Copy displayed next to previous
}

void Screen_EPD_EXT3_Fast::COG_update(uint8_t updateMode)
{
    b_waitBusy(LOW); // 150 specific
    b_sendCommand8(0x20); // Display Refresh
    digitalWrite(b_pin.panelCS, HIGH); // CS# = 1
    b_waitBusy(LOW); // 150 specific
}

void Screen_EPD_EXT3_Fast::COG_powerOff()
{
    // Empty
}
/// @endcond
//
// === End of COG section
//

//
// === Class section
//
Screen_EPD_EXT3_Fast::Screen_EPD_EXT3_Fast(eScreen_EPD_EXT3_t eScreen_EPD_EXT3, pins_t board)
{
    u_eScreen_EPD_EXT3 = eScreen_EPD_EXT3;
    b_pin = board;
    u_newImage = 0; // nullptr
}

void Screen_EPD_EXT3_Fast::begin()
{
    u_codeExtra = (u_eScreen_EPD_EXT3 >> 16) & 0xff;
    u_codeSize = (u_eScreen_EPD_EXT3 >> 8) & 0xff;
    u_codeType = u_eScreen_EPD_EXT3 & 0xff;
    _screenColourBits = 2; // BWR and BWRY

    // Configure board
    b_begin(b_pin, FAMILY_SMALL, 0);

    // Checks
    if (not(u_codeExtra & FEATURE_FAST))
    {
        Serial.println();
        Serial.println("* ERROR - Not a fast update screen");
        while (true);
    }
    if (not(u_codeExtra & FEATURE_HIGH_DEFINITION))
    {
        Serial.println();
        Serial.println("* ERROR - Not a high-definition screen");
        while (true);
    }

    switch (u_codeSize)
    {
        case 0x15: // 1.50" and 1.52"

            _screenSizeV = 200; // vertical = wide size
            _screenSizeH = 200; // horizontal = small size
            _screenDiagonal = 152;
            break;

        default:

            break;
    } // u_codeSize

    u_bufferDepth = _screenColourBits; // 2 colours
    u_bufferSizeV = _screenSizeV; // vertical = wide size
    u_bufferSizeH = _screenSizeH / 8; // horizontal = small size 112 / 8, 1 bit per pixel

    // Force conversion for two unit16_t multiplication into uint32_t.
    // Actually for 1 colour; BWR requires 2 pages.
    u_pageColourSize = (uint32_t)u_bufferSizeV * (uint32_t)u_bufferSizeH;

    // u_frameSize = u_pageColourSize, except for 9.69 and 11.98
    u_frameSize = u_pageColourSize;
    // 9.69 and 11.98 combine two half-screens, hence two frames with adjusted size

#if defined(BOARD_HAS_PSRAM) // ESP32 PSRAM specific case

    if (u_newImage == 0)
    {
        static uint8_t * _newFrameBuffer;
        _newFrameBuffer = (uint8_t *) ps_malloc(u_pageColourSize * u_bufferDepth);
        u_newImage = (uint8_t *) _newFrameBuffer;
    }

#else // default case

    if (u_newImage == 0)
    {
        static uint8_t * _newFrameBuffer;
        _newFrameBuffer = new uint8_t[u_pageColourSize * u_bufferDepth];
        u_newImage = (uint8_t *) _newFrameBuffer;
    }

#endif // ESP32 BOARD_HAS_PSRAM

    memset(u_newImage, 0x00, u_pageColourSize * u_bufferDepth);

    // Initialise the /CS pins
    pinMode(b_pin.panelCS, OUTPUT);
    digitalWrite(b_pin.panelCS, HIGH); // CS# = 1

    // New generic solution
    pinMode(b_pin.panelDC, OUTPUT);
    pinMode(b_pin.panelReset, OUTPUT);
    pinMode(b_pin.panelBusy, INPUT); // All Pins 0

    // Initialise Flash /CS as HIGH
    if (b_pin.flashCS != NOT_CONNECTED)
    {
        pinMode(b_pin.flashCS, OUTPUT);
        digitalWrite(b_pin.flashCS, HIGH);
    }

    // Initialise slave panel /CS as HIGH
    if (b_pin.panelCSS != NOT_CONNECTED)
    {
        pinMode(b_pin.panelCSS, OUTPUT);
        digitalWrite(b_pin.panelCSS, HIGH);
    }

    // Initialise slave Flash /CS as HIGH
    if (b_pin.flashCSS != NOT_CONNECTED)
    {
        pinMode(b_pin.flashCSS, OUTPUT);
        digitalWrite(b_pin.flashCSS, HIGH);
    }

    // Initialise SD-card /CS as HIGH
    if (b_pin.cardCS != NOT_CONNECTED)
    {
        pinMode(b_pin.cardCS, OUTPUT);
        digitalWrite(b_pin.cardCS, HIGH);
    }

    // Initialise SPI
    _settingScreen = {4000000, MSBFIRST, SPI_MODE0};

#if defined(ENERGIA)

    SPI.begin();
    SPI.setBitOrder(_settingScreen.bitOrder);
    SPI.setDataMode(_settingScreen.dataMode);
    SPI.setClockDivider(SPI_CLOCK_MAX / min(SPI_CLOCK_MAX, _settingScreen.clock));

#else

#if defined(ARDUINO_XIAO_ESP32C3)

    // Board Xiao ESP32-C3 crashes if pins are specified.
    SPI.begin(8, 9, 10); // SCK MISO MOSI

#elif defined(ARDUINO_NANO_ESP32)

    // Board Arduino Nano ESP32 arduino_nano_nora v2.0.11
    SPI.begin();

#elif defined(ARDUINO_ARCH_ESP32)

    // Board ESP32-Pico-DevKitM-2 crashes if pins are not specified.
    SPI.begin(14, 12, 13); // SCK MISO MOSI

#else

    SPI.begin();

#endif // ARDUINO_ARCH_ESP32

    SPI.beginTransaction(_settingScreen);

#endif // ENERGIA

    // Reset
    b_reset(5, 5, 10, 5, 5); // small

    // Check after reset
    // if (eScreen_EPD_EXT3 == eScreen_EPD_EXT3_150_0J_Wide)
    if ((u_codeSize == 0x15) and (u_codeType == 0x4A))
    {
        if (digitalRead(b_pin.panelBusy) == HIGH)
        {
            Serial.println();
            Serial.println("* ERROR - Incorrect type for 1.50-Wide");
            while (true);
        }
    }

    // Check type and get tables
    COG_getUserData(); // nothing sent to panel

    // Standard
    hV_Screen_Buffer::begin();

    setOrientation(0);
    if (f_fontMax() > 0)
    {
        f_selectFont(0);
    }
    f_fontSolid = false;

    _penSolid = false;
    u_invert = false;

    // Report
    Serial.println(formatString("= Screen %s %ix%i", WhoAmI().c_str(), screenSizeX(), screenSizeY()));
    Serial.println(formatString("= PDLS %s v%i.%i.%i", SCREEN_EPD_EXT3_VARIANT, SCREEN_EPD_EXT3_RELEASE / 100, (SCREEN_EPD_EXT3_RELEASE / 10) % 10, SCREEN_EPD_EXT3_RELEASE % 10));

    clear();
}

String Screen_EPD_EXT3_Fast::WhoAmI()
{
    char work[64] = {0};
    u_WhoAmI(work);

    return formatString("iTC %i.%02i\"%s", _screenDiagonal / 100, _screenDiagonal % 100, work);
}

uint8_t Screen_EPD_EXT3_Fast::flushMode(uint8_t updateMode)
{
    updateMode = checkTemperatureMode(updateMode);

    switch (updateMode)
    {
        case UPDATE_FAST:
        case UPDATE_GLOBAL:

            _flushFast();
            break;

        default:

            Serial.println("* PDLS - UPDATE_NONE invoked");
            break;
    }

    return updateMode;
}

void Screen_EPD_EXT3_Fast::flush()
{
    flushMode(UPDATE_FAST);
}

void Screen_EPD_EXT3_Fast::_flushFast()
{
    // Configure
    COG_initial(UPDATE_FAST);

    // Send image data
    COG_sendImageDataFast();

    // Update
    COG_update(UPDATE_FAST);

    COG_powerOff();
}

void Screen_EPD_EXT3_Fast::clear(uint16_t colour)
{
    if (colour == myColours.grey)
    {
        for (uint16_t i = 0; i < u_bufferSizeV; i++)
        {
            uint16_t pattern = (i % 2) ? 0b10101010 : 0b01010101;
            for (uint16_t j = 0; j < u_bufferSizeH; j++)
            {
                u_newImage[i * u_bufferSizeH + j] = pattern;
            }
        }
    }
    else if ((colour == myColours.white) xor u_invert)
    {
        // physical black 00
        memset(u_newImage, 0x00, u_pageColourSize);
    }
    else
    {
        // physical white 10
        memset(u_newImage, 0xff, u_pageColourSize);
    }
}

void Screen_EPD_EXT3_Fast::regenerate()
{
    clear(myColours.black);
    flush();
    delay(100);

    clear(myColours.white);
    flush();
    delay(100);
}

void Screen_EPD_EXT3_Fast::_setPoint(uint16_t x1, uint16_t y1, uint16_t colour)
{
    // Orient and check coordinates are within screen
    // _orientCoordinates() returns false = success, true = error
    if (_orientCoordinates(x1, y1) == RESULT_ERROR)
    {
        return;
    }

    // Convert combined colours into basic colours
    bool flagOdd = ((x1 + y1) % 2 == 0);

    if (colour == myColours.grey)
    {
        if (flagOdd)
        {
            colour = myColours.black; // black
        }
        else
        {
            colour = myColours.white; // white
        }
    }

    // Coordinates
    uint32_t z1 = _getZ(x1, y1);
    uint16_t b1 = _getB(x1, y1);

    // Basic colours
    if ((colour == myColours.white) xor u_invert)
    {
        // physical black 00
        bitClear(u_newImage[z1], b1);
    }
    else if ((colour == myColours.black) xor u_invert)
    {
        // physical white 10
        bitSet(u_newImage[z1], b1);
    }
}

void Screen_EPD_EXT3_Fast::_setOrientation(uint8_t orientation)
{
    _orientation = orientation % 4;
}

bool Screen_EPD_EXT3_Fast::_orientCoordinates(uint16_t & x, uint16_t & y)
{
    bool _flagResult = RESULT_ERROR; // false = success, true = error
    switch (_orientation)
    {
        case 3: // checked, previously 1

            if ((x < _screenSizeV) and (y < _screenSizeH))
            {
                x = _screenSizeV - 1 - x;
                _flagResult = RESULT_SUCCESS;
            }
            break;

        case 2: // checked

            if ((x < _screenSizeH) and (y < _screenSizeV))
            {
                x = _screenSizeH - 1 - x;
                y = _screenSizeV - 1 - y;
                swap(x, y);
                _flagResult = RESULT_SUCCESS;
            }
            break;

        case 1: // checked, previously 3

            if ((x < _screenSizeV) and (y < _screenSizeH))
            {
                y = _screenSizeH - 1 - y;
                _flagResult = RESULT_SUCCESS;
            }
            break;

        default: // checked

            if ((x < _screenSizeH) and (y < _screenSizeV))
            {
                swap(x, y);
                _flagResult = RESULT_SUCCESS;
            }
            break;
    }

    return _flagResult;
}

uint32_t Screen_EPD_EXT3_Fast::_getZ(uint16_t x1, uint16_t y1)
{
    uint32_t z1 = 0;
    // According to 11.98 inch Spectra Application Note
    // at http:// www.pervasivedisplays.com/LiteratureRetrieve.aspx?ID=245146

    z1 = (uint32_t)x1 * u_bufferSizeH + (y1 >> 3);

    return z1;
}

uint16_t Screen_EPD_EXT3_Fast::_getB(uint16_t x1, uint16_t y1)
{
    uint16_t b1 = 0;

    b1 = 7 - (y1 % 8);

    return b1;
}

uint16_t Screen_EPD_EXT3_Fast::_getPoint(uint16_t x1, uint16_t y1)
{
    // Orient and check coordinates are within screen
    // _orientCoordinates() returns false = success, true = error
    if (_orientCoordinates(x1, y1) == RESULT_ERROR)
    {
        return 0;
    }

    uint16_t _result = 0;
    uint8_t _value = 0;

    // Coordinates
    uint32_t z1 = _getZ(x1, y1);
    uint16_t b1 = _getB(x1, y1);

    _value = bitRead(u_newImage[z1], b1);
    _value <<= 4;
    _value &= 0b11110000;

    // red = 0-1, black = 1-0, white 0-0
    switch (_value)
    {
        case 0x10:

            _result = myColours.black;
            break;

        default:

            _result = myColours.white;
            break;
    }

    return _result;
}
//
// === End of Class section
//

//
// === Touch section
//

//
// === End of Touch section
//

