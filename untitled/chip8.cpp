// chip8.cpp
#include "chip8.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <random>

// Constants for SDL rendering
const int SCREEN_WIDTH = 640;    // 64 pixels * 10 for scaling
const int SCREEN_HEIGHT = 320;   // 32 pixels * 10 for scaling
const int PIXEL_SIZE = 10;       // Size of each pixel when rendered

Chip8::Chip8() {
    // Initialize program counter to start at 0x200
    pc = 0x200;
    I = 0;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;

    // Clear memory, registers, stack, display, and keypad
    std::memset(memory, 0, sizeof(memory));
    std::memset(V, 0, sizeof(V));
    std::memset(stack, 0, sizeof(stack));
    std::memset(display, 0, sizeof(display));
    std::memset(keypad, 0, sizeof(keypad));

    loadFontset();
    initializeGraphics();
}

Chip8::~Chip8() {
    cleanupGraphics();
}

void Chip8::loadFontset() {
    // CHIP-8 font set (each character is 5 bytes)
    uint8_t fontset[80] = {
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

    // Load fontset into memory starting at 0x50
    for (int i = 0; i < 80; ++i) {
        memory[0x50 + i] = fontset[i];
    }
}

void Chip8::loadROM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        std::vector<char> buffer(size);

        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), size);
        file.close();

        // Load ROM into memory starting at 0x200
        for (int i = 0; i < size; ++i) {
            memory[0x200 + i] = buffer[i];
        }

        std::cout << "ROM loaded successfully.\n";
    } else {
        std::cerr << "Failed to open ROM file: " << filename << "\n";
    }
}

void Chip8::initializeGraphics() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << "\n";
        exit(1);
    }

    window = SDL_CreateWindow("CHIP-8 Emulator",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << "\n";
        exit(1);
    }
}

void Chip8::cleanupGraphics() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Chip8::render() {
    // Clear screen with black color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Set draw color to white for pixels
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            if (display[y * DISPLAY_WIDTH + x] == 1) {
                SDL_Rect pixel = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}

void Chip8::handleInput(SDL_Event& event) {
    bool pressed = (event.type == SDL_KEYDOWN);
    bool released = (event.type == SDL_KEYUP);
    if (pressed || released) {
        bool state = pressed;
        switch (event.key.keysym.sym) {
            case SDLK_x: keypad[0x0] = state; break;
            case SDLK_1: keypad[0x1] = state; break;
            case SDLK_2: keypad[0x2] = state; break;
            case SDLK_3: keypad[0x3] = state; break;
            case SDLK_q: keypad[0x4] = state; break;
            case SDLK_w: keypad[0x5] = state; break;
            case SDLK_e: keypad[0x6] = state; break;
            case SDLK_a: keypad[0x7] = state; break;
            case SDLK_s: keypad[0x8] = state; break;
            case SDLK_d: keypad[0x9] = state; break;
            case SDLK_z: keypad[0xA] = state; break;
            case SDLK_c: keypad[0xB] = state; break;
            case SDLK_4: keypad[0xC] = state; break;
            case SDLK_r: keypad[0xD] = state; break;
            case SDLK_f: keypad[0xE] = state; break;
            case SDLK_v: keypad[0xF] = state; break;
            default: break; // Ignore other keys
        }
    }
}

void Chip8::emulateCycle() {
    fetchOpcode();
    executeOpcode();
}

void Chip8::fetchOpcode() {
    opcode = memory[pc] << 8 | memory[pc + 1];
}

void Chip8::executeOpcode() {
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: // 00E0: Clear the display
                    std::memset(display, 0, sizeof(display));
                    pc += 2;
                    break;
                case 0x00EE: // 00EE: Return from subroutine
                    if (sp == 0) {
                        std::cerr << "Stack underflow!\n";
                        exit(1);
                    }
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
                default:
                    std::cerr << "Unknown opcode [0x0000]: " << std::hex << opcode << "\n";
                    pc += 2;
            }
            break;

        case 0x1000: // 1NNN: Jump to address NNN
            pc = opcode & 0x0FFF;
            break;

        case 0x2000: // 2NNN: Call subroutine at NNN
            if (sp >= STACK_SIZE) {
                std::cerr << "Stack overflow!\n";
                exit(1);
            }
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000: { // 3XNN: Skip next instruction if VX == NN
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            if (V[X] == NN) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }

        case 0x4000: { // 4XNN: Skip next instruction if VX != NN
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            if (V[X] != NN) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }

        case 0x5000: { // 5XY0: Skip next instruction if VX == VY
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            if (V[X] == V[Y]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }

        case 0x6000: { // 6XNN: Set VX to NN
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] = NN;
            pc += 2;
            break;
        }

        case 0x7000: { // 7XNN: Add NN to VX (no carry)
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            V[X] += NN;
            pc += 2;
            break;
        }

        case 0x8000: {
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            switch (opcode & 0x000F) {
                case 0x0000: // 8XY0: Set VX = VY
                    V[X] = V[Y];
                    pc += 2;
                    break;
                case 0x0001: // 8XY1: Set VX = VX | VY
                    V[X] |= V[Y];
                    pc += 2;
                    break;
                case 0x0002: // 8XY2: Set VX = VX & VY
                    V[X] &= V[Y];
                    pc += 2;
                    break;
                case 0x0003: // 8XY3: Set VX = VX ^ VY
                    V[X] ^= V[Y];
                    pc += 2;
                    break;
                case 0x0004: { // 8XY4: Add VY to VX, set VF = carry
                    uint16_t sum = V[X] + V[Y];
                    V[0xF] = (sum > 0xFF) ? 1 : 0;
                    V[X] = sum & 0xFF;
                    pc += 2;
                    break;
                }
                case 0x0005: { // 8XY5: Subtract VY from VX, set VF = NOT borrow
                    V[0xF] = (V[X] >= V[Y]) ? 1 : 0;
                    V[X] -= V[Y];
                    pc += 2;
                    break;
                }
                case 0x0006: { // 8XY6: Shift VX right by 1, set VF to least significant bit before shift
                    V[0xF] = V[X] & 0x1;
                    V[X] >>= 1;
                    pc += 2;
                    break;
                }
                case 0x0007: { // 8XY7: Set VX = VY - VX, set VF = NOT borrow
                    V[0xF] = (V[Y] >= V[X]) ? 1 : 0;
                    V[X] = V[Y] - V[X];
                    pc += 2;
                    break;
                }
                case 0x000E: { // 8XYE: Shift VX left by 1, set VF to most significant bit before shift
                    V[0xF] = (V[X] & 0x80) >> 7;
                    V[X] <<= 1;
                    pc += 2;
                    break;
                }
                default:
                    std::cerr << "Unknown opcode [0x8000]: " << std::hex << opcode << "\n";
                    pc += 2;
            }
            break;
        }

        case 0x9000: { // 9XY0: Skip next instruction if VX != VY
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            if (V[X] != V[Y]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }

        case 0xA000: { // ANNN: Set I to address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        }

        case 0xB000: { // BNNN: Jump to address NNN + V0
            pc = (opcode & 0x0FFF) + V[0];
            break;
        }

        case 0xC000: { // CXNN: Set VX to random byte AND NN
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t NN = opcode & 0x00FF;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            V[X] = dis(gen) & NN;
            pc += 2;
            break;
        }

        case 0xD000: { // DXYN: Draw sprite at (VX, VY) with height N
            uint8_t X = V[(opcode & 0x0F00) >> 8];
            uint8_t Y = V[(opcode & 0x00F0) >> 4];
            uint8_t height = opcode & 0x000F;
            V[0xF] = 0;

            for (int row = 0; row < height; ++row) {
                uint8_t spriteByte = memory[I + row];
                for (int col = 0; col < 8; ++col) {
                    uint8_t spritePixel = spriteByte & (0x80 >> col);
                    int displayX = (X + col) % DISPLAY_WIDTH;
                    int displayY = (Y + row) % DISPLAY_HEIGHT;
                    int displayIndex = displayY * DISPLAY_WIDTH + displayX;

                    if (spritePixel) {
                        if (display[displayIndex] == 1) {
                            V[0xF] = 1; // Collision detected
                        }
                        display[displayIndex] ^= 1; // XOR the pixel
                    }
                }
            }
            pc += 2;
            break;
        }

        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E: { // EX9E: Skip next instruction if key VX is pressed
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    if (keypad[V[X]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                }
                case 0x00A1: { // EXA1: Skip next instruction if key VX is not pressed
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    if (!keypad[V[X]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                }
                default:
                    std::cerr << "Unknown opcode [0xE000]: " << std::hex << opcode << "\n";
                    pc += 2;
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: { // FX07: Set VX to value of delay timer
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    V[X] = delay_timer;
                    pc += 2;
                    break;
                }
                case 0x000A: { // FX0A: Wait for a key press, store in VX
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    bool keyPressed = false;
                    for (int i = 0; i < 16; ++i) {
                        if (keypad[i]) {
                            V[X] = i;
                            keyPressed = true;
                            break;
                        }
                    }
                    if (keyPressed) {
                        pc += 2;
                    }
                    // If no key is pressed, do not increment PC to wait
                    break;
                }
                case 0x0015: { // FX15: Set delay timer to VX
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    delay_timer = V[X];
                    pc += 2;
                    break;
                }
                case 0x0018: { // FX18: Set sound timer to VX
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    sound_timer = V[X];
                    pc += 2;
                    break;
                }
                case 0x001E: { // FX1E: Add VX to I
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    I += V[X];
                    pc += 2;
                    break;
                }
                case 0x0029: { // FX29: Set I to location of sprite for character in VX
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    I = V[X] * 5; // Each character is 5 bytes
                    pc += 2;
                    break;
                }
                case 0x0033: { // FX33: Store BCD of VX in memory locations I, I+1, I+2
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    uint8_t value = V[X];
                    memory[I]     = value / 100;
                    memory[I + 1] = (value / 10) % 10;
                    memory[I + 2] = value % 10;
                    pc += 2;
                    break;
                }
                case 0x0055: { // FX55: Store V0 to VX in memory starting at I
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i <= X; ++i) {
                        memory[I + i] = V[i];
                    }
                    pc += 2;
                    break;
                }
                case 0x0065: { // FX65: Read V0 to VX from memory starting at I
                    uint8_t X = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i <= X; ++i) {
                        V[i] = memory[I + i];
                    }
                    pc += 2;
                    break;
                }
                default:
                    std::cerr << "Unknown opcode [0xF000]: " << std::hex << opcode << "\n";
                    pc += 2;
            }
            break;

        default:
            std::cerr << "Unknown opcode: " << std::hex << opcode << "\n";
            pc += 2;
    }
}

void Chip8::updateTimers() {
    if (delay_timer > 0) {
        --delay_timer;
    }
    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "BEEP!\n"; // add sounds later
        }
        --sound_timer;
    }
}
