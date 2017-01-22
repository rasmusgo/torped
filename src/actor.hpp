#pragma once

class Actor
{
public:
    Actor();
    virtual ~Actor();
    virtual void Update(unsigned int ticks);
    virtual void Draw();
private:
};
