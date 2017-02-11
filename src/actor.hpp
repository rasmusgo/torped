#pragma once

class GameApp;

class Actor
{
public:
    Actor();
    virtual ~Actor();
    virtual void Update(unsigned int ticks);
    virtual void Draw(GameApp& gameapp);
private:
};
