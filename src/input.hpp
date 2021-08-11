#pragma once

#include "pch.hpp"

#include "profiler.hpp"

class GameApp;

class AppInput
{
public:
    AppInput();
    ~AppInput();
    int HandleGameEvents(GameApp& gameapp);
    void UpdateMice(GameApp& gameapp);
    void FlushMice(GameApp& gameapp);
    int GetLastActiveDevice() { return last_active_device; }

private:
    bool discard_mouse_event = false;
    int num_mice;
    int last_active_device = -1;
    std::vector<float> mouse_sens;
    std::vector<SDL_Joystick *> joysticks;

    bool fatal_error;
    Profiler profiler;
};
