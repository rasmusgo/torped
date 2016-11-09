#include <angelscript.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "aswrap.h"
#include "logging.h"

void AsMessageCallback(const asSMessageInfo *msg, void *param)
{
    loguru::Verbosity verbosity = loguru::NamedVerbosity::Verbosity_ERROR;
    if( msg->type == asMSGTYPE_WARNING )
        verbosity = loguru::NamedVerbosity::Verbosity_WARNING;
    else if( msg->type == asMSGTYPE_INFORMATION )
        verbosity = loguru::NamedVerbosity::Verbosity_INFO;

    //printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
    loguru::StreamLogger(verbosity, msg->section, msg->row) << msg->message;
}

void AsWrapConsoleCmds(asIScriptEngine *as)
{
    // The script compiler will send any compiler messages to the callback function
    as->SetMessageCallback(asFUNCTION(AsMessageCallback), 0, asCALL_CDECL);
}

void AsRunFile(asIScriptEngine *as, const char *filename)
{
    /*
    // Compile the script code
    r = CompileScript(as);
    if( r < 0 ) return -1;
    */
}
