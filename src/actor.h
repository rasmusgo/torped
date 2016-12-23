#ifndef ACTOR_H
#define ACTOR_H

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
    virtual void Update();
    virtual void Draw();
private:
};

#endif // ACTOR_H
