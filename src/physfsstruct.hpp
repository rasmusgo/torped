#pragma once

#include <SDL.h>
#include <physfs.h>

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
