#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <map>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "vectormath.h"

class PhyInstance;

struct SpringState
{
    REAL l, k ,d ,s;
};

class Pose
{
public:
    std::map<int, SpringState> springStates;
    void Load(const TiXmlHandle &hPose, PhyInstance *inst);
    void Apply(PhyInstance *inst);
    void Blend(PhyInstance *inst, float a, float b);
};
