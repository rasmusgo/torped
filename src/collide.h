#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "vectormath.h"
#include "physics.h"

struct TraceResult
{
    PhyPoint* point;
    Vec3r delta;
    REAL length;
    REAL dist;
};

void TraceLine(const Vec3r &pos, const Vec3r &dir, const REAL max_dist, Physics *phys, TraceResult *res);
