// main.cpp
#include "chip8.h"
#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // Check if ROM filename is provided
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>\n";
        return 1;
    }


    Chip8 chip8;

    // Load the specified ROM
    std::string romPath = argv[1];
    chip8.loadROM(romPath);

    // Main emulation loop
    bool running = true;
    SDL_Event event;

    // To manage timer updates at 60Hz
    Uint32 timerLastUpdate = SDL_GetTicks();

    while (running) {
        // Handle SDL events (input and window events)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            chip8.handleInput(event);
        }

        // Emulate one cycle (fetch, decode, execute)
        chip8.emulateCycle();

        // Update timers at 60Hz
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - timerLastUpdate >= 16) { // Approximately 60Hz
            chip8.updateTimers();
            timerLastUpdate = currentTime;
        }

        // Render the display
        chip8.render();

        // Delay to limit frame rate (~60Hz)
        SDL_Delay(16);
    }


    return 0;
}
