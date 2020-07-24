#pragma once

#include "pch.hpp"

#include "vectormath.hpp"

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
