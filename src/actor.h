#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

class Actor
{
public:
    Actor();
    virtual ~Actor();
    virtual void Update(unsigned int ticks);
    virtual void Draw();
private:
};
