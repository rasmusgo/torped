#ifndef ALSTRUCT_H
#define ALSTRUCT_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <al/al.h>
#include <al/alc.h>
//#include <alut/alut.h>
#include <vector>
#include <string>
#include <map>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "vectormath.h"

class AlStruct
{
public:
    ALCcontext *context;
    ALCdevice *device;

    bool InitAl();
    void QuitAl();
    bool LoadSound(const char filename[]);
    ALuint AddSound(const char filename[], const Vec3r &pos, const Vec3r &vel=Vec3r(0,0,0), float gain=1);

private:
    bool LoadOGG(const char filename[], std::vector < char > &buffer, ALenum &format, ALsizei &freq);
    std::map<std::string, ALuint> buffers;
    std::vector<ALuint> sources;
};

namespace App
{
	extern AlStruct al;
}

#endif //ALSTRUCT_H
