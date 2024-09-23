// chip8.h
#ifndef CHIP8_H
#define CHIP8_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>

class Chip8 {
public:
    // Constants for CHIP-8 specifications
    static const int MEMORY_SIZE = 4096;
    static const int REGISTER_COUNT = 16;
    static const int STACK_SIZE = 16;
    static const int DISPLAY_WIDTH = 64;
    static const int DISPLAY_HEIGHT = 32;

    // Constructor and Destructor
    Chip8();
    ~Chip8();

    // Public Methods
    void loadROM(const std::string& filename);
    void initializeGraphics();
    void cleanupGraphics();
    void render();
    void handleInput(SDL_Event& event);
    void emulateCycle();
    void updateTimers();

private:
    // CHIP-8 Components
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[REGISTER_COUNT];          // Registers V0 to VF
    uint16_t I;                         // Index register
    uint16_t pc;                        // Program counter
    uint16_t stack[STACK_SIZE];
    uint16_t sp;                        // Stack pointer
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT]; // 64x32 display buffer
    uint8_t keypad[16];                 // CHIP-8 keypad
    uint16_t opcode;                    // Current opcode

    // SDL Components
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Private Methods
    void loadFontset();
    void fetchOpcode();
    void executeOpcode();
};

#endif // CHIP8_H
