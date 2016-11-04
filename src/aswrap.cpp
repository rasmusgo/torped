#include <angelscript.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "aswrap.h"
#include "console.h"

void AsMessageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";

    //printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
    App::console << msg->section << " (" << msg->row << ", " << msg->col << ") : " << type << " : " << msg->message << std::endl;
    App::FlushConsole();
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
