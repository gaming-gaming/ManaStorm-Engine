// Store the "Thank you" message in static memory
// I will be flummoxed if anyone ever finds this in the games memory dump
const char thankyou[32] __attribute__((used, section(".rodata"))) = "Thank you for playing our game!";

#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <windows.h>

// #include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GameMeta.hpp"
#include "game_process.hpp"
#include "config/EngineConfig.hpp"
#include "input/input.hpp"
#include "graphics/render.hpp"
#include "graphics/TextureManager.hpp"
#include "graphics/window.hpp"

const std::string CONFIG_FILE_NAME = "engine_config.json";
const std::string META_FILE_NAME = "game_meta.json";
const double TICK_RATE = 1.0 / 60.0; // Game runs at 60 ticks per second, interpolated rendering
int framerate_limit = 120; // TODO: Make this configurable later, if 0 then uncapped

int main() {
    // Initialize game development libraries
    if (!glfwInit()) {
        MessageBoxA(nullptr, "Failed to initialize GLFW.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }
    /*
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        MessageBoxA(nullptr, "Failed to initialize SDL.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }
    */

    // Load engine configuration
    EngineConfig engine_config;
    engine_config.loadFromFile("../../dat/" + CONFIG_FILE_NAME);

    // Load game metadata
    GameMeta game_meta;
    game_meta.loadFromFile("../../" + META_FILE_NAME);
    int window_width = engine_config.getResolutionWidth();
    int window_height = engine_config.getResolutionHeight();

    // Window title
    const std::string title_str = game_meta.getTitle();

    // Main window
    Window window(
        title_str.c_str(),
        // engine_config.getDisplayMode() == "fullscreen",
        false,
        window_width,
        window_height
    );

    // Set up the materials path for the renderer
    SetMaterialsPath("../../" + game_meta.getDirectory() + "/materials");

    // Set the TMAP file
    if (!setTmap("../../" + game_meta.getDirectory() + "/maps/test.tmap")) {
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
        window.update(window_width, window_height);

        // Update game logic at fixed tick rate
        while (lag >= TICK_RATE) {
            runGameProcess(TICK_RATE);
            lag -= TICK_RATE;
        }

        // Sleep to maintain frame rate limit
        if (framerate_limit > 0) {
            Sleep(static_cast<DWORD>(1000 / framerate_limit));
        }
    }

    return 0;
}