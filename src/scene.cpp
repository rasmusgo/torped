#include "pch.hpp"

#if defined(__APPLE__) || defined(WIN32)
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <SDL.h>

#include "actor.hpp"
#include "individual.hpp"
#include "physstruct.hpp"
#include "player.hpp"
#include "scene.hpp"
#include "vectormath.hpp"
#include "world.hpp"

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
        this->data = data;
    }

    ~ProtectedData()
    {
    }

    T Get()
    {
        T tmp;
        {
            std::lock_guard<std::mutex> lock(mutex);
            tmp = data;
        }
        return tmp;
    }

    void Set(T data)
    {
        std::lock_guard<std::mutex> lock(mutex);
        this->data = data;
    }
private:
    T data;
    std::mutex mutex;
};

int StartScene(void *data)
{
    return ((Scene*)data)->Loop();
}

Scene::Scene()
{
    flags = 0;
    realTicks = SDL_GetTicks();
    physicsTicks = 0;
}

Scene::~Scene()
{
}

void Scene::Spawn(const char *filename)
{
    std::lock_guard<std::mutex> lock(actors_mutex);
    actors.push_back(new Individual(filename));
}

int Scene::GetFlags()
{
    int tmp;
    std::lock_guard<std::mutex> lock(flags_mutex);
    tmp = flags;
    return tmp;
}

void Scene::AddFlags(SceneFlags tmp)
{
    std::lock_guard<std::mutex> lock(flags_mutex);
    flags |= tmp;
}

void Scene::RemoveFlags(SceneFlags tmp)
{
    std::lock_guard<std::mutex> lock(flags_mutex);
    flags &= ~tmp;
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
            UpdatePhysics();
            // FIXME: Player should be moved to Scene
            App::player.Fly(App::player.vel.x*0.005, App::player.vel.y*0.005, App::player.vel.z*0.005);
            UpdateOpenAL();
            UpdateActors();

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

void Scene::UpdatePhysics()
{
    std::lock_guard<std::mutex> lock(phyInstances_lock);

    for (auto& it : phyInstances)
    {
        it->phys->UpdateForces();

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
    for (auto it = phyInstances.begin(); it != phyInstances.end(); ++it)
        for (auto it2 = it+1; it2 != phyInstances.end(); ++it2)
            (*it)->phys->TestBounds(*((*it2)->phys), 0.1);

    for (auto& it : phyInstances)
        it->phys->UpdateVelocitiesAndPositions();

    static unsigned int last_crashhandling = 0;
    if (physicsTicks % 500 == last_crashhandling)
    {
        for (auto& it : phyInstances)
            it->CrashHandling();
    }

    for (auto& it : phyInstances)
    {
        if (it->phys->insane)
        {
            it->CrashHandling();
            last_crashhandling = physicsTicks % 500;
        }
    }
}

void Scene::UpdateOpenAL()
{
    std::lock_guard<std::mutex> lock(phyInstances_lock);
    // Sound emitters
    for (auto& it : phyInstances)
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
            REAL gain = mean.Length()*50 -0.5;
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

    // Listener
    Mat3x3r mat(App::player.rot);
    ALfloat orientation[] = { float(mat.vec1.x), float(mat.vec1.y), float(mat.vec1.z),
                              float(mat.vec3.x), float(mat.vec3.y), float(mat.vec3.z) };
    alListenerfv(AL_ORIENTATION, orientation);
    alListener3f(AL_POSITION, App::player.pos.x, App::player.pos.y, App::player.pos.z);
    alListener3f(AL_VELOCITY, App::player.vel.x, App::player.vel.y, App::player.vel.z);
}

void Scene::UpdateActors()
{
    std::lock_guard<std::mutex> lock(actors_mutex);
    for (auto it = actors.begin(); it != actors.end(); ++it)
        (*it)->Update(physicsTicks);
}
