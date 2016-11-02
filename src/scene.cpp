#include <AL/al.h>
#include <AL/alc.h>
//#include <alut/alut.h>

#include <SDL.h>
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include <vector>
#include <list>
#include <string>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "vectormath.h"
#include "player.h"
#include "physstruct.h"
#include "scene.h"
#include "world.h"

template <class T>
class ProtectedData
{
public:
    ProtectedData()
    {
        mutex = SDL_CreateMutex();
    }

    ProtectedData(const T data)
    {
        mutex = SDL_CreateMutex();
        this->data = data;
    }

    ~ProtectedData()
    {
        SDL_DestroyMutex(mutex);
    }

    T Get()
    {
        T tmp;
        SDL_LockMutex(mutex);
        tmp = data;
        SDL_UnlockMutex(mutex);
        return tmp;
    }

    void Set(T data)
    {
        SDL_LockMutex(mutex);
        this->data = data;
        SDL_UnlockMutex(mutex);
    }
private:
    T data;
    SDL_mutex *mutex;
};

int StartScene(void *data)
{
    return ((Scene*)data)->Loop();
}

Scene::Scene()
{
    flags_mutex = SDL_CreateMutex();
    actors_mutex = SDL_CreateMutex();
    flags = 0;
    physicsTicks = 0;
}

Scene::~Scene()
{
    SDL_DestroyMutex(flags_mutex);
    SDL_DestroyMutex(actors_mutex);
}

void Scene::AddActor(Actor*)
{

}

int Scene::GetFlags()
{
    int tmp;
    SDL_LockMutex(flags_mutex);
    tmp = flags;
    SDL_UnlockMutex(flags_mutex);
    return tmp;
}

void Scene::AddFlags(SceneFlags tmp)
{
    SDL_LockMutex(flags_mutex);
    flags |= tmp;
    SDL_UnlockMutex(flags_mutex);
}

void Scene::RemoveFlags(SceneFlags tmp)
{
    SDL_LockMutex(flags_mutex);
    flags &= ~tmp;
    SDL_UnlockMutex(flags_mutex);
}

int Scene::Loop()
{
    while (1)
    {
        while (1)
        {
            int tmp = GetFlags();
            /*
            SDL_LockMutex(game_physics_messages_lock);
            bool draw = game_physics_messages.draw;
            bool quit = game_physics_messages.quit;
            SDL_UnlockMutex(game_physics_messages_lock);
            */
            if (tmp & SCENE_QUIT)
                return 1;
            if (tmp & SCENE_DRAW)
                SDL_Delay(1);
            else
                break;
        }

        Uint32 dt = realTicks;
        realTicks = SDL_GetTicks();
        dt = realTicks - dt;

        int inv_speed = 1;
        if ( SDL_GetModState() & KMOD_SHIFT )
            inv_speed = 4;
        if ( SDL_GetModState() & KMOD_ALT )
            inv_speed *= 8;

        // Decide target physics time
        Uint32 targetTicks = physicsTicks + dt/inv_speed;
        // Step back real time to manage precision loss of target time
        realTicks -= dt%inv_speed;

        while ((Sint32)(targetTicks - physicsTicks) > 0)
        {
            SDL_LockMutex(phyInstances_lock);
            typeof(phyInstances.begin()) it, it2;

            for (it = phyInstances.begin(); it != phyInstances.end(); ++it)
            {
                it->phys->DoFrame1();

                if (App::world)
                {
                    App::world->CollideWorld(*it->phys);
                }
                else
                {
                    it->phys->CollideFloor();
                }
            }

            // TODO: Fixa smartare test typ dela upp rymden i regioner och testa regionerna för sig
            for (it = phyInstances.begin(); it != phyInstances.end(); ++it)
                for (it2 = it+1; it2 != phyInstances.end(); ++it2)
                    it->phys->TestBounds(*(it2->phys), 0.1);

            for (it = phyInstances.begin(); it != phyInstances.end(); ++it)
                it->phys->DoFrame2();

            static unsigned int last_crashhandling = 0;
            if (physicsTicks % 500 == last_crashhandling)
            {
                for (it = phyInstances.begin(); it != phyInstances.end(); ++it)
                    it->CrashHandling();
            }

            for (it = phyInstances.begin(); it != phyInstances.end(); ++it)
            {
                if (it->phys->insane)
                {
                    it->CrashHandling();
                    last_crashhandling = physicsTicks % 500;
                }
            }

            // Sound emitters
            for (it = phyInstances.begin(); it != phyInstances.end(); ++it)
            {
                for (unsigned int j = 0; j < it->phys->sounds_count; ++j)
                {
                    PhySound *sound = it->phys->sounds + j;

                    Vec3r mean(0,0,0);
                    for (int i = 0; i < PHY_SOUND_SAMPLES; ++i) // *it = sound->buffer; it < sound->buffer+32; ++it)
                    {
                        mean += sound->buffer[i];
                    }
                    mean /= PHY_SOUND_SAMPLES;

                    REAL buffer[PHY_SOUND_SAMPLES];
                    REAL max = 0;
                    for (int i = 0; i < PHY_SOUND_SAMPLES; ++i)
                    {
                        REAL vel = (sound->buffer[(sound->i + i) % PHY_SOUND_SAMPLES] - mean).SqrLength();

                        if (vel > max)
                            max = vel;

                        buffer[i] = vel;
                    }

                    REAL d0 = buffer[1] - buffer[0];
                    REAL d1 = 0;
                    REAL freq = 0;
                    for (int i = 2; i < PHY_SOUND_SAMPLES; ++i)
                    {
                        d1 = buffer[i] - buffer[i-1];
                        if ( (d1 > 0) != (d0 > 0) )
                            ++freq;
                        d0 = d1;
                    }

                    //REAL gain = max * 1000000;
                    //REAL pitch = freq * (10.0 / PHY_SOUND_SAMPLES);
                    ALint state;
                    alGetSourcei(sound->source, AL_SOURCE_STATE, &state);
                    REAL gain = mean.Length()*100 -1;
                    if (gain > 0)
                    {
                        if (state != AL_PLAYING)
                            alSourcePlay(sound->source);

                        REAL pitch = gain; //freq * (10.0 / PHY_SOUND_SAMPLES);
                        alSourcef(sound->source, AL_PITCH, pitch);
                        alSourcef(sound->source, AL_GAIN, gain);
                        alSource3f(sound->source, AL_POSITION, sound->p1->pos.x, sound->p1->pos.y, sound->p1->pos.z);
                        alSource3f(sound->source, AL_VELOCITY, mean.x, mean.y, mean.z);
                        //App::console << "pitch: " << pitch << " gain: " << gain << std::endl;
                    }
                    else if (state == AL_PLAYING)
                        alSourceStop(sound->source);
                }
            }

            // FIXME: Player should be moved to Scene
            App::player.Fly(App::player.vel.x*0.005, App::player.vel.y*0.005, App::player.vel.z*0.005);

            alListener3f(AL_POSITION, App::player.pos.x, App::player.pos.y, App::player.pos.z);
            alListener3f(AL_VELOCITY, App::player.vel.x, App::player.vel.y, App::player.vel.z);

            SDL_UnlockMutex(phyInstances_lock);

            if (SDL_GetTicks()-realTicks > 100)
            {
                targetTicks = physicsTicks;
                break;
            }

            physicsTicks += 1;
        }

        AddFlags(SCENE_DRAW);
    }

    return 1;
}
