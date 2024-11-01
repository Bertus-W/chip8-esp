#pragma once
#include "view.h"

#define KEYSIZE 40
#define CHIP8_DISPLAY_HEIGHT 32
#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_PIXEL_SIZE 3

#define EMULATOR_HZ 500
#define DISPLAY_HZ 15
#define TIMER_HZ 60
#define KEYBOARD_HZ 30

const char keys[4][4] = {{'1', '2', '3', 'C'},
                         {'4', '5', '6', 'D'},
                         {'7', '8', '9', 'E'},
                         {'A', '0', 'B', 'F'}};

class Chip8View : public View
{
public:
    Chip8View();
    void init();
    void step(TFT_eSPI &, BBCapTouch &);
    void reset();
    void drawKeyboard(TFT_eSPI &tft);
    bool readTouchedKeys(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY);
    void resetTouchedKeys();
    void drawDisplay(TFT_eSPI &tft, bool display[CHIP8_DISPLAY_HEIGHT][CHIP8_DISPLAY_WIDTH]);
    void chip8Loop();

    char *programData = nullptr;
    size_t programSize = 0;
    CHIP8Emulator *chip8;

private:
    bool touchedKeys[4][4];
    TOUCHINFO ti;
};