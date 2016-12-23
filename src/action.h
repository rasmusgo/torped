#ifndef ACTION_H
#define ACTION_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

// Abstract base class
class Action
{
public:
    virtual ~Action() { };
    virtual void Run(float value) = 0;
};

#endif //ACTION_H
