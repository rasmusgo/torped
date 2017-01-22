#pragma once

#include <SDL.h>
#include <vector>
#include <string>
#include <map>

#include "profiler.hpp"
#include "action.hpp"

namespace App
{
    // From input.cpp
    void InitInput();
	int HandleGameEvents();
	void UpdateMice();
    void FlushMice();
	void ParseEvent(std::string event, float value, bool silent);
	void Bind(std::string event, Action *action);
	void UnBind(std::string event);

    extern bool discard_mouse_event;
    extern int num_mice;
    extern int last_active_device;
    extern std::vector<float> mouse_sens;
    extern std::vector<SDL_Joystick *> joysticks;
    extern std::map<std::string, Action*> actions_table;

    // From gameapp.cpp
    #ifndef APPMODE
    #define APPMODE
    enum AppMode
    {
        MENU = 1,
        GAME
    };
    #endif

	void SwitchMode(AppMode mode);

    extern bool fatal_error;
    extern Profiler profiler;

} // namespace App
