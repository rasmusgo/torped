#pragma once

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "SDL_thread.h"

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "actor.h"

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

    std::list<Actor*> actors;
    std::list<Physics*> physics;
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
