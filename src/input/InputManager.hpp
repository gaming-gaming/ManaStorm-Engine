#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

struct ControllerButtonsState {
    bool move_forward = false;
    bool move_left = false;
    bool move_backward = false;
    bool move_right = false;
    bool use = false;
    bool jump = false;
};

struct ControllerAnalogState {
    float left_x = 0.0f;
    float left_y = 0.0f;
    float right_x = 0.0f;
    float right_y = 0.0f;
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
};

struct ControllerState {
    ControllerButtonsState buttons;
    ControllerAnalogState analog;
};

void InitializeInput();
void ShutdownInput();
ControllerState ProcessInput();

#endif // INPUT_MANAGER_HPP