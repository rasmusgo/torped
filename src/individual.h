#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <map>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "actor.h"
#include "physics.h"
#include "physstruct.h"

class Individual: public Actor
{
public:
    Individual(const char *filename);
    virtual ~Individual();
    virtual void Update(unsigned int ticks);
    virtual void Draw();
    virtual void SetupLua();

private:
    PhyInstance *inst;
    lua_State *L;
    static std::map<lua_State*, Individual*> lua_state_to_individual;

    friend int LuaMethodPose(lua_State* L);
    friend int LuaMethodMotor(lua_State* L);
};
