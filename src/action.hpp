#pragma once

// Abstract base class
class Action
{
public:
    virtual ~Action() { };
    virtual void Run(float value) = 0;
};
