#pragma once

#include <map>
#include <cmath>
#include <sstream>
#include <string>

#include "console.hpp"
#include "gameapp.hpp"
#include "physics.hpp"
#include "physstruct.hpp"
#include "profiler.hpp"
#include "texture.hpp"

class GameApp;

class AppMenu
{
public:
    void ClearConsole();
	void SetupMenu(GameApp& gameapp);
	void DrawMenu(GameApp& gameapp);
	int MenuFrame(GameApp& gameapp);

private:
    std::vector<std::string> commands = {""};
    Texture consoleBackground;
    Texture luaIcon;
};
