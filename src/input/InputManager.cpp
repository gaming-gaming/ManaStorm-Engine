#include "InputManager.hpp"

#include <windows.h>
#include <SDL2/SDL.h>

static SDL_GameController* g_controller = nullptr;

void InitializeInput() {
    // Initialize controller if available
    if (SDL_NumJoysticks() > 0) {
        g_controller = SDL_GameControllerOpen(0);
        if (g_controller) {
            // Controller opened successfully
        }
    }
}

void ShutdownInput() {
    if (g_controller) {
        SDL_GameControllerClose(g_controller);
        g_controller = nullptr;
    }
}

ControllerState ProcessInput() {
    ControllerState state;
    
    // CRITICAL: Pump SDL events so controller state updates
    SDL_PumpEvents();
    
    // Keyboard input
    #ifdef _WIN32
        if (GetAsyncKeyState('W') & 0x8000) {
            state.buttons.move_forward = true;
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            state.buttons.move_backward = true;
        }
        if (GetAsyncKeyState('A') & 0x8000) {
            state.buttons.move_left = true;
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            state.buttons.move_right = true;
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            state.buttons.jump = true;
        }
        if (GetAsyncKeyState('E') & 0x8000) {
            state.buttons.use = true;
        }
    #endif

    // Game controller input
    if (g_controller) {
        // Buttons
        state.buttons.jump |= SDL_GameControllerGetButton(g_controller, SDL_CONTROLLER_BUTTON_A);
        state.buttons.use |= SDL_GameControllerGetButton(g_controller, SDL_CONTROLLER_BUTTON_X);

        // Analog sticks
        state.analog.left_x = SDL_GameControllerGetAxis(g_controller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
        state.analog.left_y = SDL_GameControllerGetAxis(g_controller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
        state.analog.right_x = SDL_GameControllerGetAxis(g_controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
        state.analog.right_y = SDL_GameControllerGetAxis(g_controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
        state.analog.left_trigger = SDL_GameControllerGetAxis(g_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
        state.analog.right_trigger = SDL_GameControllerGetAxis(g_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
    }
    
    return state;
}