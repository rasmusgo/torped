#include <SDL.h>
//#include "SDL_sound.h"
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <errno.h>
#include <physfs.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "alstruct.h"
#include "logging.h"
#include "physfsrwops.h"

using namespace std;

namespace App
{
	AlStruct al;
}
/*
static bool LoadSound(const char filename[])
{
    // Is it already loaded?
    if ( buffers.find(filename) != buffers.end() )
        return true;

    SDL_RWops *rw = PHYSFSRWOPS_openRead(filename);

    if (rw == NULL)
    {
        return false;
    }

    ALenum err;
    ALenum format;
    ALsizei freq;
    vector<char> buffer_data;
    if ( !LoadOGG(filename, buffer_data, format, freq) )
    {
        App::console << "LoadOGG failed for: " << filename << std::endl;
        return false;
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    if (buffer == 0)
    {
        err = alGetError();
        App::console << "alGenBuffers() failed: " << alGetString(err) << std::endl;
        return false;
    }

    alBufferData(buffer, format, &buffer_data[0], static_cast<ALsizei>(buffer_data.size()), freq);
    if ((err = alGetError()))
    {
        alDeleteBuffers(1, &buffer);
        App::console << "alBufferData() failed: " << alGetString(err) << std::endl;
        return false;
    }

    // Hurray!
    buffers[filename] = buffer;
    return true;
}
*/
static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    int a = PHYSFS_read( (PHYSFS_file*)datasource, ptr, size, nmemb );
    if (a == -1)
    {
        // set errorcode
        errno = EIO;
        return 0;
    }
    return a;
}

static int seek_func(void *datasource, ogg_int64_t offset, int whence)
{
/*
    switch (whence)
    {
    case SEEK_SET:
        PHYSFS_seek( (PHYSFS_file*)datasource, offset );
        return 0;
    case SEEK_CUR:
        PHYSFS_seek( (PHYSFS_file*)datasource,
                     PHYSFS_tell((PHYSFS_file*)datasource) + offset );
        return 0;
    case SEEK_END:
        PHYSFS_seek
    }
*/
    return -1;
}

static int close_func(void *datasource)
{
    PHYSFS_close( (PHYSFS_file*)datasource );
    return 0;
}

static long tell_func(void *datasource)
{
    return PHYSFS_tell( (PHYSFS_file*)datasource );
}

static ov_callbacks callbacks = {read_func, seek_func, close_func, tell_func};

bool AlStruct::InitAl()
{
    ALCenum err;

    //Open device
    //device = alcOpenDevice((ALCubyte*)("DirectSound3D"));
    device = alcOpenDevice(NULL);
    if ((err = alcGetError(device)) != ALC_NO_ERROR)
    {
        LOG_S(ERROR) << "alcOpenDevice() failed: " << alcGetString(device, err);
        return false;
    }

    //Create context(s)
    context = alcCreateContext(device, NULL);
    if ((err = alcGetError(device)) != ALC_NO_ERROR)
    {
        LOG_S(ERROR) << "alcCreateContext() failed: " << alcGetString(device, err);
        return false;
    }

    //Set active context
    alcMakeContextCurrent(context);
    if ((err = alcGetError(device)) != ALC_NO_ERROR)
    {
        LOG_S(ERROR) << "alcMakeContextCurrent() failed: " << alcGetString(device, err);
        return false;
    }

    //Register extensions
    //*/
    return true;
}

void AlStruct::QuitAl()
{
    for (__typeof(sources.begin()) iter=sources.begin(); iter != sources.end(); ++iter)
    {
        if ( !alIsSource(*iter) )
            continue;
        alSourceStop(*iter);
        alDeleteSources(1, &(*iter));
    }
    sources.clear();

    for (__typeof(buffers.begin()) iter=buffers.begin(); iter != buffers.end(); ++iter)
    {
        if ( !alIsBuffer(iter->second) )
            continue;
        alDeleteBuffers(1, &(iter->second));
    }
    buffers.clear();

    //Disable context
    alcMakeContextCurrent(NULL);
    //Release context(s)
    alcDestroyContext(context);
    //Close device
    alcCloseDevice(device);
    //*/
}

#define BUFFER_SIZE     32768       // 32 KB buffers

bool AlStruct::LoadOGG(const char filename[], vector < char > &buffer, ALenum &format, ALsizei &freq)
{
    // Open for binary reading
    PHYSFS_file *f = PHYSFS_openRead(filename);

    if (f == NULL)
    {
        LOG_S(ERROR) << "fopen() failed for: " << filename << ": " << PHYSFS_getLastError();
        return false;
    }

    // Try opening the given file
    OggVorbis_File oggFile;

    if (ov_open_callbacks((void*)f, &oggFile, NULL, 0, callbacks) )
    {
        LOG_S(ERROR) << "ov_open() failed for: " << filename;
        return false;
    }

    // Get some information about the OGG file
    vorbis_info *pInfo = ov_info(&oggFile, -1);

    // Check the number of channels... always use 16-bit samples
    if (pInfo->channels == 1)
        format = AL_FORMAT_MONO16;
    else
        format = AL_FORMAT_STEREO16;

    // The frequency of the sampling rate
    freq = pInfo->rate;

    // Keep reading until all is read
    long bytes;
    char array[BUFFER_SIZE];                // Local fixed size array
    int endian = (SDL_BYTEORDER == SDL_BIG_ENDIAN); // 0 for Little-Endian, 1 for Big-Endian
    int bitStream;
    do
    {
        // Read up to a buffer's worth of decoded sound data
        bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);

        if (bytes < 0)
        {
            ov_clear(&oggFile);
            LOG_S(ERROR) << "ov_read() failed for: " << filename;
            return false;
        }
        // end if

        // Append to end of buffer
        buffer.insert(buffer.end(), array, array + bytes);
    }
    while (bytes > 0);

    // Clean up!
    ov_clear(&oggFile);
    return true;
}

bool AlStruct::LoadSound(const char filename[])
{
    // Is it already loaded?
    if ( buffers.find(filename) != buffers.end() )
        return true;

    ALenum err;
    ALenum format;
    ALsizei freq;
    vector<char> buffer_data;
    if ( !LoadOGG(filename, buffer_data, format, freq) )
    {
        LOG_S(ERROR) << "LoadOGG failed for: " << filename;
        return false;
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    if (buffer == 0)
    {
        err = alGetError();
        LOG_S(ERROR) << "alGenBuffers() failed: " << alGetString(err);
        return false;
    }

    alBufferData(buffer, format, &buffer_data[0], static_cast<ALsizei>(buffer_data.size()), freq);
    if ((err = alGetError()))
    {
        alDeleteBuffers(1, &buffer);
        LOG_S(ERROR) << "alBufferData() failed: " << alGetString(err);
        return false;
    }

    // Hurray!
    buffers[filename] = buffer;
    return true;
}

ALuint AlStruct::AddSound(const char filename[], const Vec3r &pos, const Vec3r &vel, float gain)
{
    // load the file if nessesary
    if ( !LoadSound(filename) )
        return 0; // error has already been set by LoadSound

    ALenum err;

    ALuint source;
    alGenSources(1, &source);
    if ((err = alGetError()))
    {
        LOG_S(ERROR) << "alGenSources() failed: " << alGetString(err);
        return 0;
    }
    sources.push_back(source);

    alSourcei(source, AL_BUFFER, buffers[filename]);
    if ((err = alGetError()))
    {
        LOG_S(ERROR) << "alSourcei() failed: " << alGetString(err);
        return 0;
    }

    alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
    alSource3f(source, AL_VELOCITY, vel.x, vel.y, vel.z);
    if ((err = alGetError()))
    {
        LOG_S(ERROR) << "alSourcefv() failed: " << alGetString(err);
        return 0;
    }

    alSourcef(source, AL_GAIN, gain);
    if ((err = alGetError()))
    {
        LOG_S(ERROR) << "alSourcef() failed: " << alGetString(err);
        return 0;
    }

    //alSourcePlay(source);
    return source;
}
