#include <SDL.h>
#include "manymouse/manymouse.h"

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "input.h"
#include "console.h"

namespace App
{
    bool discard_mouse_event = false;
    int num_mice;
    std::vector<float> mouse_sens;
    std::vector<SDL_Joystick *> joysticks;
    std::map<std::string, Action*> actions_table;

    void InitInput()
    {
        num_mice = ManyMouse_Init();
        App::console << "Found " << num_mice << (num_mice == 1? " mouse:": " mice:") << std::endl;
        App::FlushConsole();

        mouse_sens.clear();
        for (int i = 0; i < num_mice; i++)
        {
            mouse_sens.push_back(1);
        	App::console << " Mouse #" << i << ":" << std::endl;
        	App::console << "  Name: " << ManyMouse_DeviceName(i) << std::endl;
        	FlushConsole();
        }

        SDL_JoystickEventState(SDL_ENABLE);
        App::console << "Found " << SDL_NumJoysticks() << " joystick" << (SDL_NumJoysticks() == 1? ":": "s:") << std::endl;
        App::FlushConsole();

        joysticks.clear();
        for( int i=0; i < SDL_NumJoysticks(); i++ )
        {
        	joysticks.push_back(SDL_JoystickOpen(i));
        	if (joysticks.back() != NULL)
        	{
                App::console << " Joystick #" << i << ":" << std::endl;
                App::console << "  Name: " << SDL_JoystickName(i) << std::endl;
                App::console << "  Number of Axes: " << SDL_JoystickNumAxes(joysticks.back()) << std::endl;
                App::console << "  Number of Buttons: " << SDL_JoystickNumButtons(joysticks.back()) << std::endl;
                App::console << "  Number of Balls: " << SDL_JoystickNumBalls(joysticks.back()) << std::endl;
                App::console << "  Number of Hats: " << SDL_JoystickNumHats(joysticks.back()) << std::endl;
                App::FlushConsole();
        	}
        	else
        	{
        		App::console << " Failed to open joystick #" << i << std::endl;
        		FlushConsole();
        	}
        }
    }

    void FlushMice()
    {
        // format: mouse#_[rel/abs][x/y]
        // ex: "mouse0_relx"
        ManyMouseEvent event;
        while (ManyMouse_PollEvent(&event))
        {
            if ( event.type == MANYMOUSE_EVENT_DISCONNECT )
                ParseEvent("_disconnect", 1, true);
        }

        discard_mouse_event = true;
    }

    void UpdateMice()
    {
        // format: mouse#_[rel/abs][x/y]
        // ex: "mouse0_relx"
        ManyMouseEvent event;
        while (ManyMouse_PollEvent(&event))
        {
            if (int(event.device) >= num_mice)
                continue;

            float value = 0;
            std::stringstream event_name;

            event_name << "mouse" << event.device;

            switch (event.type)
            {
            case MANYMOUSE_EVENT_RELMOTION:
                if (event.item == 0)
                    event_name << "_relx";
                else if (event.item == 1)
                    event_name << "_rely";
                // TODO: Add a generic filtering of events and make it scriptable
                value = event.value * mouse_sens[event.device];
                break;
            case MANYMOUSE_EVENT_ABSMOTION:
                if (event.item == 0)
                    event_name << "_absx";
                else if (event.item == 1)
                    event_name << "_absy";
                value = static_cast<float>(event.value - event.minval) / static_cast<float>(event.maxval - event.minval);
                break;
            case MANYMOUSE_EVENT_BUTTON:
                event_name << "_button" << event.item;
                value = event.value;
                break;
            case MANYMOUSE_EVENT_SCROLL:
                event_name << "_wheel" << event.item;
                value = event.value;
                break;
            case MANYMOUSE_EVENT_DISCONNECT:
                event_name << "_disconnect";
                value = 1;
                break;
            default:
                // should not happen
                continue;
            }
            ParseEvent(event_name.str(), value, true);
        }
    }

    int HandleGameEvents()
    {
        /*
        static const int numevents = 1024;
        static SDL_Event events[numevents];
        int count = SDL_PeepEvents(events, numevents, SDL_GETEVENT, SDL_ALLEVENTS);
        SDL_Event *it = events;
        SDL_Event *end = events + count;
        for (; it < end; ++it)
        */
        int events_count = 0;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ++events_count;
//#define event (*it)
            switch (event.type)
            {
            case SDL_JOYAXISMOTION:
                {
                    std::stringstream name;
                    name << "joy" << (int)event.jaxis.which;
                    name << "_axis" << (int)event.jaxis.axis;
                    if (event.jaxis.value < 0)
                    	ParseEvent( name.str(), event.jaxis.value/32768.0, true);
                    else
                    	ParseEvent( name.str(), event.jaxis.value/32767.0, true);
                }
                break;
            case SDL_JOYBALLMOTION:
                {
                    std::stringstream name;
                    name << "joy" << (int)event.jball.which;
                    name << "_ball" << (int)event.jball.ball;
                    ParseEvent( name.str() + "_relx", event.jball.xrel, true);
                    ParseEvent( name.str() + "_rely", event.jball.yrel, true);
                }
                break;
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                {
                    std::stringstream name;
                    name << "joy" << (int)event.jbutton.which;
                    name << "_button" << (int)event.jbutton.button;
                    if (event.jbutton.state == SDL_PRESSED)
                    	ParseEvent( name.str(), 1, false);
                    else
                    	ParseEvent( name.str(), 0, true);
                }
                break;
            case SDL_JOYHATMOTION:
                {
                    std::stringstream name;
                    name << "joy" << (int)event.jhat.which;
                    name << "_hat" << (int)event.jhat.hat;
                    ParseEvent( name.str(), event.jhat.value, false);
                    // TODO: create separate events for different directions
                }
                break;
            case SDL_MOUSEMOTION:
                if (discard_mouse_event)
                {
                    discard_mouse_event = false;
                    break;
                }
                ParseEvent("mouse_relx", event.motion.xrel, true);
                ParseEvent("mouse_rely", event.motion.yrel, true);
                //player.MoveMouse(event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                {
                    int value = (event.button.state == SDL_PRESSED) ? 1: 0;
                    switch (event.button.button)
                    {
                    case SDL_BUTTON_LEFT:
                        ParseEvent("mouse_button0", value, !value);
                        break;
                    case SDL_BUTTON_RIGHT:
                        ParseEvent("mouse_button1", value, !value);
                        break;
                    case SDL_BUTTON_MIDDLE:
                        ParseEvent("mouse_button2", value, !value);
                        break;
                    case SDL_BUTTON_WHEELUP:
                        // NOTE: wheel event will be sent twice here but not for individual mice
                        ParseEvent("mouse_wheel0", value, !value);
                        break;
                    case SDL_BUTTON_WHEELDOWN:
                        ParseEvent("mouse_wheel0", -value, !value);
                        break;
                    }
                }
                break;
            case SDL_KEYDOWN:
                //App::console << "Pressed " << SDL_GetKeyName(event.key.keysym.sym) << endl;
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    SwitchMode(MENU);
                    return 1;
                }
                ParseEvent( SDL_GetKeyName(SDLKey(event.key.keysym.sym)), 1, false);
                break;
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    break;
                ParseEvent( SDL_GetKeyName(SDLKey(event.key.keysym.sym)), 0, true);
                break;
            case SDL_QUIT:
                return 0;
            default:
                break;
            }
//#undef event
        }

        std::stringstream ss;
        ss << "SDL events: " << events_count;
        profiler.AddTime( ss.str().c_str() );

        UpdateMice();
        profiler.AddTime("ManyMouse events");

        return -1;
    }

    void ParseEvent(std::string event, float value, bool silent)
    {
        if ( actions_table.find(event) != actions_table.end() )
        {
            actions_table[event]->Run(value);
        }
        else if (!silent || developermode > 1)
            App::console << event << " is not bound" << std::endl;
    }

    void Bind(std::string event, Action *action)
    {
        UnBind(event);
        actions_table[event] = action;
    }

    void UnBind(std::string event)
    {
        if ( actions_table.find(event) != actions_table.end() )
        {
            delete actions_table[event];
            actions_table.erase(event);
        }
    }

} // namespace App

