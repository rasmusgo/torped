extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "gameapp.h"
#include "input.h"
#include "player.h"
#include "physstruct.h"
#include "console.h"
#include "luawrap.h"
#include "physfsstruct.h"

void LuaReportErrors(lua_State *L, int status)
{
    if ( status != 0 )
    {
        App::console << "Lua error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1); // remove error message
    }
}

void LuaRunFile(lua_State *L, const char *filename)
{
    char *buffer;
    PhysFSLoadFile(filename, buffer);
    if (buffer == NULL)
    {
        App::console << "Failed to open \"" << filename << "\".";
        return;
    }

    LuaReportErrors( L, luaL_loadstring(L, buffer) || lua_pcall(L, 0, 0, 0) );
    delete [] buffer;
    buffer = NULL;
}

class LuaAction: public Action
{
public:
    LuaAction(lua_State *L, const char *s)
    {
        this->L = L;
        this->s = s;
    }
    virtual ~LuaAction()
    {
    }
    virtual void Run(float value)
    {
        std::stringstream ss;
        ss << "value = " << value << "\n" << s;

        LuaReportErrors( L, luaL_loadstring(L, ss.str().c_str()) || lua_pcall(L, 0, 0, 0) );

        //luaL_dostring(L, ss.str().c_str());
    }

private:
    lua_State *L;
    std::string s;
};

void LuaWrapConsoleCmds(lua_State* L)
{
    lua_register(L, "clear",       LuaCmdClear);
    lua_register(L, "load",        LuaCmdLoad);
    lua_register(L, "save",        LuaCmdSave);
    lua_register(L, "spawn",       LuaCmdSpawn);
    lua_register(L, "reloadvideo", LuaCmdReloadVideo);
    lua_register(L, "screenshot",  LuaCmdScreenshot);
    lua_register(L, "bind",        LuaCmdBind);
    lua_register(L, "unbind",      LuaCmdUnBind);
    lua_register(L, "quit",        LuaCmdQuit);
    lua_register(L, "player",      LuaCmdPlayer);
    lua_register(L, "print",       LuaCmdPrint);
    lua_register(L, "exec",        LuaCmdExec);
    lua_register(L, "dofile",      LuaCmdExec);
}

int LuaCmdClear(lua_State* L)
{
    App::ClearWorld();
    return 0; // number of return values
}

int LuaCmdLoad(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    App::LoadWorld(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdSave(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    App::SaveWorld(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdSpawn(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    lua_Number ret = int(PhyInstance::InsertPhysXML(lua_tostring(L, -n)) != NULL);
    lua_pushnumber(L, ret);
    return 1; // number of return values
}

int LuaCmdReloadVideo(lua_State* L)
{
    App::ReloadVideo();
    return 0; // number of return values
}

int LuaCmdScreenshot(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    App::SaveScreenshot(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdBind(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    if (n == 2)
        App::Bind( lua_tostring(L, -n), new LuaAction(L, lua_tostring(L, -n+1)) );
    return 0; // number of return values
}

int LuaCmdUnBind(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    App::UnBind(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdQuit(lua_State* L)
{
    App::Quit();
    return 0; // number of return values
}

int LuaCmdPlayer(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    App::player.Do(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdPrint(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    for (int i=0; i<n; ++i)
        App::console << lua_tostring(L, -n + i);
    App::console << std::endl;
    return 0; // number of return values
}

int LuaCmdExec(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    LuaRunFile(L, lua_tostring(L, -n));
    return 0; // number of return values
}
