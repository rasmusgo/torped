#pragma once

#include "vectormath.hpp"
#include "physics.hpp"

struct TraceResult
{
    PhyPoint* point;
    Vec3r delta;
    REAL length;
    REAL dist;
};

void TraceLine(const Vec3r &pos, const Vec3r &dir, const REAL max_dist, Physics *phys, TraceResult *res);
