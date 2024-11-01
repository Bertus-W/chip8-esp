#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define MEMORY_SIZE 4096
#define STACK_SIZE 16
#define PROGRAM_OFFSET 0x200
#define VX_SIZE 16
#define FONT_SIZE 80
#define FONT_START_ADDRES 0x0

class CHIP8Emulator
{
public:
    CHIP8Emulator(const char *programData, size_t programSize);
    ~CHIP8Emulator();
    void loadFont();
    void loadProgram(const char *programData, size_t programSize);
    void step();
    bool (*getDisplay())[DISPLAY_WIDTH];
    void clearDisplay();
    void handleInput(bool t_keyPresses[4][4]);
    void updateTimers();

private:
    uint8_t *memory;
    uint8_t *V;
    uint16_t *stack;
    uint16_t I;
    uint8_t DT;
    uint8_t ST;
    uint8_t *program;
    bool display[DISPLAY_HEIGHT][DISPLAY_WIDTH] = {};

    double lastTimerUpdate;
    const double timerInterval = 1.0 / 60.0;
    const int cyclesPerFrame = 10;

    static const int font[FONT_SIZE];
    bool keyPresses[16];
};
