#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "actor.hpp"

enum SceneFlags
{
    SCENE_QUIT   = 1 << 0,
    SCENE_DRAW   = 1 << 1,
    SCENE_PAUSE  = 1 << 2,
    SCENE_PAUSED = 1 << 3,
    SCENE_RESUME = 1 << 4
};

int StartScene(void *data);

class Scene
{
public:
    Scene();
    ~Scene();

    void Spawn(const char *filename);

    int GetFlags();
    void AddFlags(SceneFlags);
    void RemoveFlags(SceneFlags);

private:
    int Loop();
    void UpdatePhysics();
    void UpdateActors();
    void DumpGraphics();

    std::vector<Actor*> actors;
    std::vector<Physics*> physics;
    int flags;
    Uint32 realTicks;
    Uint32 physicsTicks;

    std::mutex flags_mutex;
    std::mutex actors_mutex;
    friend int StartScene(void *data);
};

namespace App
{
    extern Scene *scene;
}
