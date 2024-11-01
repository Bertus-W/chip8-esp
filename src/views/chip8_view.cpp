#include "chip8_view.h"

Chip8View::Chip8View()
{
}

void Chip8View::init()
{
}

void Chip8View::step(TFT_eSPI &tft, BBCapTouch &bbct)
{
    static unsigned long lastStepUpdate = millis();
    if (millis() - lastStepUpdate >= 1000 / EMULATOR_HZ)
    {
        chip8->step();
        lastStepUpdate = millis();
    }

    static unsigned long lastDisplayUpdate = millis();
    if (millis() - lastDisplayUpdate >= 1000 / DISPLAY_HZ)
    {
        bool(*display)[DISPLAY_WIDTH] = chip8->getDisplay();
        drawDisplay(tft, display);
        drawKeyboard(tft);
        lastDisplayUpdate = millis();
    }

    // Update timers (usually needed only every 60 frames)
    static unsigned long lastTimerUpdate = millis();
    if (millis() - lastTimerUpdate >= 1000 / TIMER_HZ)
    {
        chip8->updateTimers();
        lastTimerUpdate = millis();
    }

    static unsigned long lastKeyboardUpdate = millis();
    if (millis() - lastKeyboardUpdate >= 1000 / KEYBOARD_HZ)
    {
        resetTouchedKeys();
        if (bbct.getSamples(&ti))
        {
            uint16_t x = ti.x[0];
            uint16_t y = ti.y[0];
            bool quit = readTouchedKeys(tft, x, y);
            if (quit)
            {
                Serial.println("Back to file select");
                setCurrentView(ViewType::FileSelect);
                return;
            }
        }
        chip8->handleInput(touchedKeys);
        lastKeyboardUpdate = millis();
    }
}

void Chip8View::reset()
{
    free(chip8);
    free(programData);
    programSize = 0;
}

void Chip8View::drawKeyboard(TFT_eSPI &tft)
{
    uint32_t startX = tft.width() / 2 - KEYSIZE * 2;
    uint32_t startY = tft.height() - 4 * KEYSIZE;

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            int xPos = startX + KEYSIZE * x;
            int yPos = startY + KEYSIZE * y;
            if (touchedKeys[x][y])
            {
                tft.fillRect(xPos, yPos, KEYSIZE, KEYSIZE, TFT_WHITE);
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
            }
            else
            {
                tft.fillRect(xPos, yPos, KEYSIZE, KEYSIZE, TFT_BLACK);
                tft.drawRect(xPos, yPos, KEYSIZE, KEYSIZE, TFT_WHITE);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            tft.setTextSize(2);
            tft.drawString(String(keys[x][y]), xPos + KEYSIZE / 5, yPos + KEYSIZE / 5);
        }
    }
    tft.drawRect(0, TFT_HEIGHT - 35, 30, 30, TFT_WHITE);
    tft.drawString("<-", 2, TFT_HEIGHT - 28);
}

bool Chip8View::readTouchedKeys(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY)
{
    uint32_t startX = tft.width() / 2 - KEYSIZE * 2;
    uint32_t startY = tft.height() - 4 * KEYSIZE;

    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            int xPos = startX + KEYSIZE * x;
            int yPos = startY + KEYSIZE * y;
            if (xPos < touchX && touchX < (xPos + KEYSIZE) && yPos < touchY && touchY < (yPos + KEYSIZE))
            {
                touchedKeys[x][y] = true;
            }
        }
    }

    if (touchX < 30 && touchY > (TFT_HEIGHT - 35))
    {
        ;
        return true;
    }
    return false;
}

void Chip8View::resetTouchedKeys()
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            touchedKeys[x][y] = 0;
        }
    }
}

void Chip8View::drawDisplay(TFT_eSPI &tft, bool display[CHIP8_DISPLAY_HEIGHT][CHIP8_DISPLAY_WIDTH])
{
    int xOffset = tft.width() / 2 - (CHIP8_DISPLAY_WIDTH * CHIP8_PIXEL_SIZE / 2);
    int yOffset = xOffset;
    for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++)
    {
        for (int x = 0; x < CHIP8_DISPLAY_WIDTH; x++)
        {
            if (display[y][x])
            {
                tft.fillRect(xOffset + x * CHIP8_PIXEL_SIZE, yOffset + y * CHIP8_PIXEL_SIZE, CHIP8_PIXEL_SIZE, CHIP8_PIXEL_SIZE,
                             TFT_WHITE);
            }
            else
            {
                tft.fillRect(xOffset + x * CHIP8_PIXEL_SIZE, yOffset + y * CHIP8_PIXEL_SIZE, CHIP8_PIXEL_SIZE, CHIP8_PIXEL_SIZE,
                             TFT_BLACK);
            }
        }
    }
}
