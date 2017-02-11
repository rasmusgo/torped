#pragma once

#include <iostream>
#include <map>
#include <math.h>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include <manymouse.h>

#include "action.hpp"
#include "appmode.hpp"
#include "console.hpp"
#include "physstruct.hpp"
#include "physics.hpp"
#include "profiler.hpp"
#include "shader.hpp"

class lua_State;
class Scene;
class AppMenu;
class AppInput;
class AlStruct;

class GameApp
{
public:
    GameApp(int argc, char *argv[]);
    ~GameApp();

    int DoFrame();
    void LoadWorld(const char filename[]);
    void SaveWorld(const char filename[]);
    void ClearWorld();
    void ClearConsole();
    void ReloadVideo();
    void SwapBuffers();
    void SwitchMode(AppMode mode);
    void SaveScreenshot(const char filename[]);
    void SetupGame();
    void DrawGame();
    int GameFrame();
    void Quit();
    void DrawPhysics(PhyInstance &inst);
    void DrawHUD();

    int GetXRes() { return xRes; }
    int GetYRes() { return yRes; }
    bool GetFullscreen() { return fullscreen; }
    void SetFullscreen(bool enable) { fullscreen = enable; }
    Scene* GetScene() { return scene; }
    Console* GetConsole() { return &console; }

    void ParseEvent(std::string event, float value, bool silent);
    void Bind(std::string event, Action *action);
    void UnBind(std::string event);
    void FlushMice();

    void LuaRunString(const std::string& str);

private:
    // Variables
    int xRes;
    int yRes;
    bool fullscreen;
    // TODO: move world and others to better places
    AppMode appMode;

    int developermode;
    int wireframemode;
    int stereo3dmode;
    float stereo3d_depth;
    float stereo3d_focus;
    float view_depth;
    int profilermode;

    bool fatal_error = false;

    Profiler profiler;

    std::vector<std::thread> threads;

    SDL_Window* window;
    SDL_GLContext glcontext;

    Shader shader;
    std::unique_ptr<AppMenu> menu;
    std::unique_ptr<AppInput> input;
    std::unique_ptr<AlStruct> al;

    Console console;
    std::deque<std::string> consoleLines;

    lua_State* lua_console = NULL;
    std::map<std::string, Action*> actions_table;

    std::thread scene_thread;
    Scene *scene = NULL;

    std::mutex game_physics_messages_lock;
};
