// Store the "Thank you" message in static memory
// I will be flummoxed if anyone ever finds this in the games memory dump
const char thankyou[32] __attribute__((used, section(".rodata"))) = "Thank you for playing our game!";

#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <windows.h>

#define SDL_MAIN_HANDLED // Prevent SDL from overriding main()
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GameMeta.hpp"
#include "game_process.hpp"
#include "config/EngineConfig.hpp"
#include "graphics/render.hpp"
#include "graphics/TextureManager.hpp"
#include "graphics/window.hpp"
#include "input/InputManager.hpp"

const std::string CONFIG_FILE_NAME = "engine_config.json";
const std::string META_FILE_NAME = "game_meta.json";
const double TICK_RATE = 1.0 / 60.0; // Game runs at 60 ticks per second, interpolated rendering
int framerateLimit = 120; // TODO: Make this configurable later, if 0 then uncapped

int main() {
    // Initialize game development libraries
    if (!glfwInit()) {
        MessageBoxA(nullptr, "Failed to initialize GLFW.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }
    SDL_SetMainReady(); // Inform SDL that main is ready
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) { // Initialize SDL for game controller support
        MessageBoxA(nullptr, "Failed to initialize SDL.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }

    InitializeInput(); // Initialize input

    // Load engine configuration
    EngineConfig engineConfig;
    engineConfig.loadFromFile("../dat/" + CONFIG_FILE_NAME);

    // Load game metadata
    GameMeta gameMeta;
    gameMeta.loadFromFile("../" + META_FILE_NAME);
    int windowWidth = engineConfig.getResolutionWidth();
    int windowHeight = engineConfig.getResolutionHeight();

    // Window title
    const std::string titleStr = gameMeta.getTitle();

    // Main window
    Window window(
        titleStr.c_str(),
        // engineConfig.getDisplayMode() == "fullscreen",
        false,
        windowWidth,
        windowHeight
    );

    // Set up the materials path for the renderer
    SetMaterialsPath("../" + gameMeta.getDirectory() + "/materials");

    // Set the TMAP file
    if (!setTmap("../" + gameMeta.getDirectory() + "/maps/test.tmap")) {
        std::cerr << "Failed to load TMAP file." << std::endl;
        return 1;
    }

    /* Old system, don't use. Doesn't account for tick rate and frame rate being separate.
    while (!window.shouldClose()) {
        runGameProcess();
        window.update();
        glfwWaitEventsTimeout(TICK_RATE);
    }
    */

    using clock = std::chrono::high_resolution_clock;
    auto previous = clock::now();
    double lag = 0.0;
    while (!window.shouldClose()) {
        auto current = clock::now();
        std::chrono::duration<double> elapsed = current - previous;
        previous = current;
        lag += elapsed.count();

        // Process input and update window
        window.update(windowWidth, windowHeight);

        // Update game logic at fixed tick rate
        while (lag >= TICK_RATE) {
            runGameProcess(TICK_RATE);
            lag -= TICK_RATE;
        }

        // Sleep to maintain frame rate limit
        if (framerateLimit > 0) {
            Sleep(static_cast<DWORD>(1000 / framerateLimit));
        }
    }

    return 0;
}