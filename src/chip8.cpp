#include "chip8.h"

CHIP8Emulator::CHIP8Emulator(const char *programData, size_t programSize)
{
    memory = (uint8_t *)calloc(MEMORY_SIZE, sizeof(uint8_t));
    V = (uint8_t *)calloc(VX_SIZE, sizeof(uint8_t));
    stack = (uint16_t *)calloc(STACK_SIZE, sizeof(uint16_t));
    I = 0;
    DT = 0;
    ST = 0;
    program = memory + PROGRAM_OFFSET;

    loadFont();
    loadProgram(programData, programSize);
    clearDisplay();
}

CHIP8Emulator::~CHIP8Emulator()
{
    free(memory);
    free(V);
    free(stack);
}

const int CHIP8Emulator::font[FONT_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void CHIP8Emulator::loadFont()
{
    for (int i = 0; i < FONT_SIZE; i++)
    {
        memory[i] = static_cast<uint8_t>(font[i]);
    }
}

void CHIP8Emulator::loadProgram(const char *programData, size_t programSize)
{
    memcpy(program, programData, programSize);
}

void CHIP8Emulator::clearDisplay()
{
    for (int y = 0; y < DISPLAY_HEIGHT; y++)
    {
        for (int x = 0; x < DISPLAY_WIDTH; x++)
        {
            display[y][x] = false;
        }
    }
}

void CHIP8Emulator::step()
{
    uint8_t left_op = *program;
    uint8_t right_op = *(program + 1);
    uint8_t first = (left_op & 0xF0) >> 4;
    uint8_t second = left_op & 0x0F;
    uint8_t third = (right_op & 0xF0) >> 4;
    uint8_t fourth = right_op & 0x0F; // No need to shift for the low nibble
    uint16_t opcode = (left_op << 8) | (right_op);
    uint16_t last_3 = opcode & 0x0FFF;

    uint8_t vx = *(V + second);
    uint8_t vy = *(V + third);

    if (opcode == 0x00E0)
    { // CLS - Clear display
        clearDisplay();
    }
    else if (opcode == 0x00EE)
    {
        stack--;
        program = memory + *stack;
        *stack = 0;
    }
    else if (first == 0x1)
    { // JP addr - Jump to address NNN
        program = memory + last_3;
        return;
    }
    else if (first == 0x2)
    { // CALL addr
        *stack = program - memory;
        stack++;
        program = memory + last_3;
        return;
    }
    else if (first == 0x3)
    { // SE Vx, byte - Skip next instruction if Vx = kk
        if (vx == right_op)
        {
            program += 2;
        }
    }
    else if (first == 0x4)
    { // SNE Vx, byte - Skip next instruction if Vx != kk
        if (vx != right_op)
        {
            program += 2;
        }
    }
    else if (first == 0x5)
    { // SE Vx, Vy - Skip next instruction if Vx = Vy
        if (vx == vy)
        {
            program += 2;
        }
    }
    else if (first == 0x6)
    { // LD Vx, byte - Set VX = kk
        *(V + second) = right_op;
    }
    else if (first == 0x7)
    { // ADD Vx, byte - Set Vx = Vx + kk
        *(V + second) += right_op;
    }
    else if (first == 0x8)
    {                 // Arithmetic operations
        uint16_t res; // extra var for result checking

        switch (fourth)
        {
        case 0x0: // LD Vx, Vy - Set Vx = Vy
            *(V + second) = vy;
            break;
        case 0x1: // OR Vx, Vy - Set Vx = Vx OR Vy bitwise
            *(V + second) = vx | vy;
            break;
        case 0x2: // AND Vx, Vy - Set Vx = Vx AND Vy bitwise
            *(V + second) = vx & vy;
            break;
        case 0x3: // XOR Vx, Vy - Set Vx = Vx XOR Vy bitwise
            *(V + second) = vx ^ vy;
            break;
        case 0x4: // ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry
            res = ((uint16_t)vx) + ((uint16_t)vy);

            if (res > 255)
            {
                *(V + 0xF) = 1;
                res &= 0xFF;
            }
            else
            {
                *(V + 0xF) = 0;
            }

            *(V + second) = (uint8_t)res;
            break;
        case 0x5: // SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow.
            *(V + 0xF) = (vx < vy) ? 0 : 1;
            *(V + second) = vx - vy;
            break;
        case 0x6: // SHR Vx {, Vy} - Set Vx = Vx SHR 1. bitshift right (div by 2)
            *(V + 0xF) = (vx & 0b1) ? 1 : 0;
            *(V + second) = vx >> 1;
            break;
        case 0x7: // SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow
            *(V + 0xF) = (vy >= vx) ? 1 : 0;
            *(V + second) = vy - vx;
            break;
        case 0xE: // SHL Vx {, Vy} - Set Vx = Vx SHL 1. bitshift left (mult by 2)
            *(V + 0xF) = (vx >> 15 & 0b1) ? 1 : 0;
            *(V + second) = vx << 1;
            break;
        default:
            break;
        }
    }
    else if (first == 0x9)
    { // SNE Vx, Vy - Skip next instruction if Vx != Vy.
        if (vx != vy)
        {
            program += 2;
        }
    }
    else if (first == 0xA)
    { // LD I, addr - Set I to NNN
        I = last_3;
    }
    else if (first == 0xC)
    { // RND Vx, byte - Set Vx = random byte AND kk.
        *(V + second) = (rand() % 255) & right_op;
    }
    else if (first == 0xD)
    {                                 // Draw sprite (Dxyn)
        uint8_t n = fourth;           // Number of rows (N)
        uint8_t *sprite = memory + I; // Sprite data starts at I
        bool collision = false;

        for (int row = 0; row < n; row++)
        {
            uint8_t sprite_byte = *(sprite + row);

            for (int col = 0; col < 8; col++)
            {
                // Get the pixel value of the sprite (each bit in the sprite byte)
                bool sprite_pixel = (sprite_byte >> (7 - col)) & 0x1;
                int x = (vx + col) % 64; // Wrap around X (64 wide)
                int y = (vy + row) % 32; // Wrap around Y (32 high)

                // XOR the sprite pixel with the current screen pixel
                if (sprite_pixel && display[y][x])
                {
                    collision = true; // A collision happened
                }
                display[y][x] ^= sprite_pixel; // XOR the sprite onto the display
            }
        }
        *(V + 0xF) = collision ? 1 : 0; // Set VF to 1 if there was a collision
    }
    else if (first == 0xE)
    { // Keyboard stuff
        if (right_op == 0x9E)
        { // SKP Vx - Skip next if key pressed
            if (keyPresses[vx])
            {
                program += 2;
            }
        }
        else if (right_op == 0xA1)
        { // SKNP Vx - Skip next if key not pressed
            if (!keyPresses[vx])
            {
                program += 2;
            }
        }
        else
        {
            printf("E command Unknown right_op: %02x\n", right_op);
        }
    }
    else if (first == 0xF)
    { // Misc. stuff
        uint16_t val;

        switch (right_op)
        {
        case 0x07: // LD Vx, DT - Set Vx = delay timer value
            *(V + second) = DT;
            break;
        case 0x0A: // LD Vx, K - Wait for a key press, store val of the key in Vx.
            static int found = -1;
            static bool released = false;

            for (int i = 0; i < 16; i++)
            {
                if (keyPresses[i])
                {
                    found = i;
                }
                else
                {
                    if (found != -1 && !released && found == i)
                    {
                        released = true;
                    }
                }
            }
            if (found == -1 || !released)
            {
                program -= 2;
            }
            else
            {
                *(V + second) = found;
                found = -1;
                released = false;
            }
            break;
        case 0x15: // LD DT, Vx - Set delay timer = Vx.
            DT = vx;
            break;
        case 0x18: // LD ST, Vx - Set sound timer = Vx.
            ST = vx;
            break;
        case 0x1E: // ADD I, Vx - Set I = I + Vx.
            I += vx;
            break;
        case 0x29: // LD F, Vx - Set I = location of sprite for digit Vx.
            I = FONT_START_ADDRES + (5 * vx);
            break;
        case 0x33: // LD B, Vx - Store BCD of Vx in memory at I, I+1, I+2
            *(memory + I) = (char)(vx / 100);
            *(memory + I + 1) = (char)(vx % 100 / 10);
            *(memory + I + 2) = (char)(vx % 10);
            break;
        case 0x55: // LD [I], Vx - Store reg V0 - Vx in memory at location I.
            for (int i = 0; i <= second; i++)
            {
                *(memory + I + i) = *(V + i);
            }
            break;
        case 0x65: // LD Vx, [I] - Read ref V0 - Vx from memory at location I.
            for (int i = 0; i <= second; i++)
            {
                *(V + i) = *(memory + I + i);
            }
            break;
        default:
            printf("F command Unknown right_op: %02x\n", right_op);
        }
    }
    else if (opcode == 0)
    {
        exit(0);
    }
    else
    {
        printf("GENERAL Unknown opcode: %04x\n", opcode);
    }

    program += 2;
}

bool (*CHIP8Emulator::getDisplay())[DISPLAY_WIDTH]
{
    return display;
}

void CHIP8Emulator::handleInput(bool t_keyPresses[4][4])
{
    keyPresses[0x0] = t_keyPresses[1][3];
    keyPresses[0x1] = t_keyPresses[0][0];
    keyPresses[0x2] = t_keyPresses[1][0];
    keyPresses[0x3] = t_keyPresses[2][0];
    keyPresses[0x4] = t_keyPresses[0][1];
    keyPresses[0x5] = t_keyPresses[1][1];
    keyPresses[0x6] = t_keyPresses[2][1];
    keyPresses[0x7] = t_keyPresses[0][2];
    keyPresses[0x8] = t_keyPresses[1][2];
    keyPresses[0x9] = t_keyPresses[2][2];
    keyPresses[0xA] = t_keyPresses[0][3];
    keyPresses[0xB] = t_keyPresses[2][3];
    keyPresses[0xC] = t_keyPresses[3][0];
    keyPresses[0xD] = t_keyPresses[3][1];
    keyPresses[0xE] = t_keyPresses[3][2];
    keyPresses[0xF] = t_keyPresses[3][3];
}

void CHIP8Emulator::updateTimers()
{
    // Timers update logic (if needed on ESP32, could be simplified)
    if (DT > 0)
        DT--;
    if (ST > 0)
        ST--;
}
