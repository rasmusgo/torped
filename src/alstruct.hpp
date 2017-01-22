#pragma once

#ifdef __APPLE__
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
//#include <alut/alut.h>
#include <vector>
#include <string>
#include <map>

#include "vectormath.hpp"

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
