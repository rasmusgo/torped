#ifndef PHYSFSSTRUCT_H
#define PHYSFSSTRUCT_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <SDL.h>
#include <physfs.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

class AutoPHYSFS_file
{
public:
    AutoPHYSFS_file(PHYSFS_file *f)
    {
        this->f = f;
    }
    ~AutoPHYSFS_file()
    {
        if (f)
            PHYSFS_close(f);
    }
    //operator PHYSFS_file* () { return f; }

    PHYSFS_file *f;
};

unsigned int PhysFSLoadFile(const char *file, char *&buffer);

#endif // PHYSFSSTRUCT_H
