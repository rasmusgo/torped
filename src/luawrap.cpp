extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "gameapp.hpp"
#include "input.hpp"
#include "logging.hpp"
#include "luawrap.hpp"
#include "physfsstruct.hpp"
#include "physstruct.hpp"
#include "player.hpp"
#include "scene.hpp"

namespace {
    GameApp* gameapp_ptr = nullptr;
}

void LuaSetGameAppPtr(GameApp* gameapp)
{
    gameapp_ptr = gameapp;
}

void LuaReportErrors(lua_State* L, int status)
{
    if ( status != 0 )
    {
        LOG_S(ERROR) << "Lua error: " << lua_tostring(L, -1);
        lua_pop(L, 1); // remove error message
    }
}

void LuaRunString(lua_State* L, const char* buffer)
{
    LuaReportErrors( L, luaL_loadstring(L, buffer) || lua_pcall(L, 0, 0, 0) );
}

void LuaRunFile(lua_State* L, const char* filename)
{
    char *buffer; // TODO(Rasmus): Replace with RAII
    PhysFSLoadFile(filename, buffer);
    if (buffer == NULL)
    {
        LOG_S(ERROR) << "Failed to open \"" << filename << "\".";
        return;
    }

    LuaRunString(L, buffer);

    delete [] buffer;
    buffer = NULL;
}

class LuaAction: public Action
{
public:
    LuaAction(lua_State* L, const char* s)
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

LuaActor::LuaActor(const char* filename)
{
    L = luaL_newstate();
    LuaRunFile(L, filename);
}

LuaActor::~LuaActor()
{
    if (L)
        lua_close(L);
}

void LuaActor::Update(unsigned int ticks)
{
    lua_getglobal(L, "update");
    lua_pushnumber(L, ticks);
    LuaReportErrors( L, lua_pcall(L, 1, 0, 0) );
}

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
    CHECK_NOTNULL_F(gameapp_ptr);
    gameapp_ptr->ClearWorld();
    return 0; // number of return values
}

int LuaCmdLoad(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    int n = lua_gettop(L); // number of arguments
    gameapp_ptr->LoadWorld(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdSave(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    int n = lua_gettop(L); // number of arguments
    gameapp_ptr->SaveWorld(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdSpawn(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    int n = lua_gettop(L); // number of arguments
    const char* str = lua_tostring(L, -n);
    if (Scene* scene = gameapp_ptr->GetScene())
    {
        scene->Spawn(str);
    }
    else
    {
        LOG_F(WARNING, "Failed to spawn '%s': scene is NULL", str);
    }
    return 0;
    //lua_Number ret = int(PhyInstance::InsertPhysXML(lua_tostring(L, -n)) != NULL);
    //lua_pushnumber(L, ret);
    //return 1; // number of return values
}

int LuaCmdReloadVideo(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    gameapp_ptr->ReloadVideo();
    return 0; // number of return values
}

int LuaCmdScreenshot(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    int n = lua_gettop(L); // number of arguments
    gameapp_ptr->SaveScreenshot(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdBind(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    int n = lua_gettop(L); // number of arguments
    if (n == 2)
        gameapp_ptr->Bind( lua_tostring(L, -n), new LuaAction(L, lua_tostring(L, -n+1)) );
    return 0; // number of return values
}

int LuaCmdUnBind(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    int n = lua_gettop(L); // number of arguments
    gameapp_ptr->UnBind(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdQuit(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    gameapp_ptr->Quit();
    return 0; // number of return values
}

int LuaCmdPlayer(lua_State* L)
{
    CHECK_NOTNULL_F(gameapp_ptr);
    int n = lua_gettop(L); // number of arguments
    App::player.Do(lua_tostring(L, -n));
    return 0; // number of return values
}

int LuaCmdPrint(lua_State* L)
{
    std::stringstream ss;
    int n = lua_gettop(L); // number of arguments
    for (int i=0; i<n; ++i)
    {
        ss << lua_tostring(L, -n + i);
    }
    std::string s = ss.str();
    LOG_S(INFO) << s.c_str();
    return 0; // number of return values
}

int LuaCmdExec(lua_State* L)
{
    int n = lua_gettop(L); // number of arguments
    LuaRunFile(L, lua_tostring(L, -n));
    return 0; // number of return values
}
