#include "pch.hpp"

#include <SDL.h>
#include <manymouse.h>

#include "gameapp.hpp"
#include "input.hpp"
#include "logging.hpp"

void to_lower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

AppInput::AppInput()
{
    // NOTE: Joystick initialization disturbs manymouse initialization if done after ManyMouse_Init()
    SDL_JoystickEventState(SDL_ENABLE);
    LOG_S(INFO) << "Found " << SDL_NumJoysticks() << " joystick" << (SDL_NumJoysticks() == 1? ":": "s:");

    joysticks.clear();
    for( int i=0; i < SDL_NumJoysticks(); i++ )
    {
        joysticks.push_back(SDL_JoystickOpen(i));
        if (joysticks.back() != NULL)
        {
            LOG_S(INFO) << " Joystick #" << i << ":";
            LOG_S(INFO) << "  Name: " << SDL_JoystickName(joysticks.back());
            LOG_S(INFO) << "  Number of Axes: " << SDL_JoystickNumAxes(joysticks.back());
            LOG_S(INFO) << "  Number of Buttons: " << SDL_JoystickNumButtons(joysticks.back());
            LOG_S(INFO) << "  Number of Balls: " << SDL_JoystickNumBalls(joysticks.back());
            LOG_S(INFO) << "  Number of Hats: " << SDL_JoystickNumHats(joysticks.back());
        }
        else
        {
            LOG_S(INFO) << " Failed to open joystick #" << i;
        }
    }

    num_mice = ManyMouse_Init();
    LOG_S(INFO) << "Found " << num_mice << (num_mice == 1? " mouse:": " mice:");

    mouse_sens.clear();
    for (int i = 0; i < num_mice; i++)
    {
        mouse_sens.push_back(1);
        LOG_S(INFO) << " Mouse #" << i << ":";
        LOG_S(INFO) << "  Name: " << ManyMouse_DeviceName(i);
    }
}

AppInput::~AppInput()
{
    for (unsigned int i=0; i < joysticks.size(); i++)
    {
        if (joysticks[i] != NULL)
            SDL_JoystickClose(joysticks[i]);
    }
    joysticks.clear();
}

void AppInput::FlushMice(GameApp& gameapp)
{
    // format: mouse#_[rel/abs][x/y]
    // ex: "mouse0_relx"
    ManyMouseEvent event;
    while (ManyMouse_PollEvent(&event))
    {
        if ( event.type == MANYMOUSE_EVENT_DISCONNECT )
            gameapp.ParseEvent("_disconnect", 1, true);
    }

    discard_mouse_event = true;
}

void AppInput::UpdateMice(GameApp& gameapp)
{
    // format: mouse#_[rel/abs][x/y]
    // ex: "mouse0_relx"
    ManyMouseEvent event;
    while ( ManyMouse_PollEvent(&event) )
    {
        if (int(event.device) >= num_mice)
            continue;

        float value = 0;
        std::stringstream event_name;

        event_name << "mouse" << event.device;

        last_active_device = event.device;

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
        gameapp.ParseEvent(event_name.str(), value, true);
    }
}

int AppInput::HandleGameEvents(GameApp& gameapp)
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
                    gameapp.ParseEvent( name.str(), event.jaxis.value/32768.0, true);
                else
                    gameapp.ParseEvent( name.str(), event.jaxis.value/32767.0, true);
            }
            break;
        case SDL_JOYBALLMOTION:
            {
                std::stringstream name;
                name << "joy" << (int)event.jball.which;
                name << "_ball" << (int)event.jball.ball;
                gameapp.ParseEvent( name.str() + "_relx", event.jball.xrel, true);
                gameapp.ParseEvent( name.str() + "_rely", event.jball.yrel, true);
            }
            break;
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            {
                std::stringstream name;
                name << "joy" << (int)event.jbutton.which;
                name << "_button" << (int)event.jbutton.button;
                if (event.jbutton.state == SDL_PRESSED)
                    gameapp.ParseEvent( name.str(), 1, false);
                else
                    gameapp.ParseEvent( name.str(), 0, true);
            }
            break;
        case SDL_JOYHATMOTION:
            {
                std::stringstream name;
                name << "joy" << (int)event.jhat.which;
                name << "_hat" << (int)event.jhat.hat;
                gameapp.ParseEvent( name.str(), event.jhat.value, false);
                // TODO: create separate events for different directions
            }
            break;
        case SDL_MOUSEMOTION:
            if (discard_mouse_event)
            {
                discard_mouse_event = false;
                break;
            }
            gameapp.ParseEvent("mouse_relx", event.motion.xrel, true);
            gameapp.ParseEvent("mouse_rely", event.motion.yrel, true);
            //player.MoveMouse(event.motion.xrel, event.motion.yrel);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            {
                int value = (event.button.state == SDL_PRESSED) ? 1: 0;
                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    gameapp.ParseEvent("mouse_button0", value, !value);
                    break;
                case SDL_BUTTON_RIGHT:
                    gameapp.ParseEvent("mouse_button1", value, !value);
                    break;
                case SDL_BUTTON_MIDDLE:
                    gameapp.ParseEvent("mouse_button2", value, !value);
                    break;
                }
            }
            break;
        case SDL_MOUSEWHEEL:
            {
                int value = event.wheel.y;
                gameapp.ParseEvent("mouse_wheel0", value, !value);
                break;
            }
        case SDL_KEYDOWN:
            {
                //App::console << "Pressed " << SDL_GetKeyName(event.key.keysym.sym) << endl;
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    gameapp.SwitchMode(MENU);
                    return 1;
                }
                std::string key = SDL_GetKeyName(event.key.keysym.sym);
                to_lower(key);
                gameapp.ParseEvent(key, 1, false);
                break;
            }
        case SDL_KEYUP:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    break;

                std::string key = SDL_GetKeyName(event.key.keysym.sym);
                to_lower(key);
                gameapp.ParseEvent(key, 0, true);
                break;
            }
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

    UpdateMice(gameapp);
    profiler.AddTime("ManyMouse events");

    return -1;
}
