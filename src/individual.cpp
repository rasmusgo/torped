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
#include "individual.h"
#include "input.h"
#include "logging.h"
#include "luawrap.h"
#include "physfsstruct.h"
#include "physstruct.h"
#include "player.h"

std::map<lua_State*, Individual*> Individual::lua_state_to_individual;

Individual::Individual(const char *filename): inst(NULL)
{
    std::string fname = filename;

    L = luaL_newstate();
    lua_state_to_individual[L] = this;

    if ( PHYSFS_exists((fname + ".lua").c_str()) )
        LuaRunFile(L, (fname + ".lua").c_str());

    inst = PhyInstance::InsertPhysXML((fname + ".xml").c_str());
}

Individual::~Individual()
{
    if (L)
        lua_close(L);

    lua_state_to_individual.erase(L);
    L = NULL;
}

void Individual::Update(unsigned int ticks)
{
    int top = lua_gettop(L);

    lua_getglobal(L, "update");
    if ( lua_isfunction(L, -1) )
    {
        lua_pushnumber(L, ticks);
        LuaReportErrors( L, lua_pcall(L, 1, 0, 0) );
    }

    lua_settop(L, top);
}

void Individual::Draw()
{
    if (inst)
    {
        SDL_LockMutex(phyInstances_lock);
        App::DrawPhysics(*inst);
        SDL_UnlockMutex(phyInstances_lock);
    }
}

int LuaMethodPose(lua_State* L)
{
    if (Individual::lua_state_to_individual.find(L) == Individual::lua_state_to_individual.end())
        return 0; // L doesn't belong to any Individual

    PhyInstance *inst = Individual::lua_state_to_individual[L]->inst;

    int n = lua_gettop(L); // number of arguments

    if (n >= 1)
    {
        std::string str = lua_tostring(L, -n);

        float a,b;
        a = b = std::numeric_limits<float>::quiet_NaN();

        if (n >= 2)
            a = lua_tonumber(L, -n+1);
        if (n >= 3)
            b = lua_tonumber(L, -n+2);

        if (inst)
        {
            SDL_LockMutex(phyInstances_lock);

            bool fail;
            if (b == b)
                fail = !inst->UpdatePhysBlend(str.c_str(), a, b);
            else if (a == a)
                fail = !inst->UpdatePhysBlend(str.c_str(), 1.0f-a, a);
            else
                fail = !inst->UpdatePhys(str.c_str());

            SDL_UnlockMutex(phyInstances_lock);

            if (fail)
            {
                // TODO: report error
            }
        }
    }

    return 0;
}

int LuaMethodMotor(lua_State* L)
{
    if (Individual::lua_state_to_individual.find(L) == Individual::lua_state_to_individual.end())
        return 0; // L doesn't belong to any Individual

    PhyInstance *inst = Individual::lua_state_to_individual[L]->inst;

    int n = lua_gettop(L); // number of arguments

    if (n == 4)
    {
        TypeName tn;
        tn.type = "motor";
        tn.name = lua_tostring(L, -n);

        Vec3r torque(lua_tonumber(L, -n+1),
                     lua_tonumber(L, -n+2),
                     lua_tonumber(L, -n+3));

        if (inst)
        {
            SDL_LockMutex(phyInstances_lock);

            if ( inst->namesIndex.find(tn) != inst->namesIndex.end() )
                inst->phys->motors[inst->namesIndex[tn]].torque = torque * (inst->phys->time * inst->phys->time);

            SDL_UnlockMutex(phyInstances_lock);
        }
    }

    return 0; // number of return values
}

void Individual::SetupLua()
{
    lua_register(L, "Pose",        LuaMethodPose);
    lua_register(L, "Motor",       LuaMethodMotor);

    lua_register(L, "print",       LuaCmdPrint);
    lua_register(L, "exec",        LuaCmdExec);
    lua_register(L, "dofile",      LuaCmdExec);
}
