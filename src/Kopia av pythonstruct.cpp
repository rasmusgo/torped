#include <SDL/SDL_thread.h>
#include <string>
#include <python.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif
#include "pythonstruct.h"
#include "gameapp.h"
#include "console.h"

extern "C"
{
    //int mode = std::numeric_limits<int>::quiet_NaN();
#define CONSOLE_INT(var, name, funcname) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    int mode = var; \
    if ( !PyArg_ParseTuple(args, "|i:"#name, &mode) ) \
        return NULL; \
    if (mode == mode) \
        var = mode; \
    return Py_BuildValue("i", var); \
}

#define CONSOLE_FLOAT(var, name, funcname) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    float mode = var; \
    if ( !PyArg_ParseTuple(args, "|f:"#name, &mode) ) \
        return NULL; \
    if (mode == mode) \
        var = mode; \
    return Py_BuildValue("f", var); \
}

#define CONSOLECOMMAND_STRINGPARAM(func, name, funcname) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    char *temp; \
    if ( !PyArg_ParseTuple(args, "s:"#name, &temp) ) \
        return NULL; \
    func(temp); \
    Py_RETURN_NONE; \
}

#define CONSOLECOMMAND_NOPARAM(func, name, funcname) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    if ( !PyArg_ParseTuple(args, ":"#name) ) \
        return NULL; \
    func(); \
    Py_RETURN_NONE; \
}

#define BEGIN_FUNC_FLOAT(name, funcname) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    float value; \
    if ( !PyArg_ParseTuple(args, "f:"#name, &value) ) \
        return NULL;

#define BEGIN_FUNC_STRING(name, funcname) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    char *value; \
    if ( !PyArg_ParseTuple(args, "s:"#name, &value) ) \
        return NULL;

#define BEGIN_FUNC_NOPARAM(name, funcname) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    if ( !PyArg_ParseTuple(args, ":"#name) ) \
        return NULL;

CONSOLE_INT(App::developermode, "developer", EmbDeveloper)
CONSOLE_INT(App::wireframemode, "wireframe", EmbWireframe)
CONSOLE_INT(App::stereo3dmode, "stereo3d", EmbStereo3d)
CONSOLE_FLOAT(App::stereo3d_depth, "stereo3d_depth", EmbStereo3d_depth)
CONSOLE_FLOAT(App::stereo3d_focus, "stereo3d_focus", EmbStereo3d_focus)
CONSOLE_FLOAT(App::view_depth, "view_depth", EmbViewDepth)

CONSOLECOMMAND_STRINGPARAM(App::LoadWorld, "LoadWorld", EmbLoadWorld)
CONSOLECOMMAND_NOPARAM(App::ReloadVideo, "ReloadVideo", EmbReloadVideo)

BEGIN_FUNC_STRING("SaveWorld",  EmbSaveWorld) App::SaveWorld(value); Py_RETURN_NONE; }
BEGIN_FUNC_NOPARAM("ClearWorld",  EmbClearWorld) App::ClearWorld(); Py_RETURN_NONE; }
BEGIN_FUNC_NOPARAM("Clear",  EmbClearConsole) App::ClearConsole(); Py_RETURN_NONE; }

BEGIN_FUNC_STRING("Screenshot",  EmbScreenshot) App::SaveScreenshot(value); Py_RETURN_NONE; }
BEGIN_FUNC_STRING("unbind",  EmbUnBind) App::UnBind(value); Py_RETURN_NONE; }
BEGIN_FUNC_NOPARAM("quit",  EmbQuit) App::Quit(); Py_RETURN_NONE; }

BEGIN_FUNC_STRING("write",  EmbWrite) App::console << value << std::flush; Py_RETURN_NONE; }

BEGIN_FUNC_FLOAT("MoveForward",EmbPlayerMoveForward) App::m_player.vel.x = value; Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("MoveBack",   EmbPlayerMoveBack)    App::m_player.vel.x = -value; Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("MoveLeft",   EmbPlayerMoveLeft)    App::m_player.vel.y = value; Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("MoveRight",  EmbPlayerMoveRight)   App::m_player.vel.y = -value; Py_RETURN_NONE; }

BEGIN_FUNC_FLOAT("LookUp",    EmbPlayerLookUp)    App::m_player.MoveMouse(0, value); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("LookDown",  EmbPlayerLookDown)  App::m_player.MoveMouse(0, -value); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("LookLeft",  EmbPlayerLookLeft)  App::m_player.MoveMouse(-value, 0); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("LookRight", EmbPlayerLookRight) App::m_player.MoveMouse(value, 0); Py_RETURN_NONE; }

BEGIN_FUNC_FLOAT("Arm1Left",  EmbPlayerArm1Left)  App::m_player.MoveArm1(-value, 0); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("Arm1Right", EmbPlayerArm1Right) App::m_player.MoveArm1(value, 0); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("Arm1Up",    EmbPlayerArm1Up)    App::m_player.MoveArm1(0, value); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("Arm1Down",  EmbPlayerArm1Down)  App::m_player.MoveArm1(0, -value); Py_RETURN_NONE; }

BEGIN_FUNC_FLOAT("Arm2Left",  EmbPlayerArm2Left)  App::m_player.MoveArm2(-value, 0); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("Arm2Right", EmbPlayerArm2Right) App::m_player.MoveArm2(value, 0); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("Arm2Up",    EmbPlayerArm2Up)    App::m_player.MoveArm2(0, value); Py_RETURN_NONE; }
BEGIN_FUNC_FLOAT("Arm2Down",  EmbPlayerArm2Down)  App::m_player.MoveArm2(0, -value); Py_RETURN_NONE; }

BEGIN_FUNC_STRING("LoadPhy", EmbLoadPhy)
/*
    int length;
    if ( !PyArg_ParseTuple(args, "s#:LoadPhy", &filename, &length) )
        return NULL;

    // the copy of the filename will be deleted by InsertPhys
    char *copy = new char[length+1];
    memcpy(copy, filename, length);
    copy[length] = '\0';
*/
    /*
    App::threads.push_back( SDL_CreateThread(InsertPhys, copy) );
    /*/
    //if ( !InsertPhys(static_cast<void*>(filename)) )

    if ( !InsertPhys(value) )
    {
        // report error
	}
	//*/
    Py_RETURN_NONE;
}

static PyObject* EmbUpdatePhy(PyObject *self, PyObject *args)
{
    char *filename;
    float a,b;
    a = b = std::numeric_limits<float>::quiet_NaN();
    if ( !PyArg_ParseTuple(args, "s|ff:UpdatePhy", &filename, &a, &b) )
        return NULL;

    if (phyInstances.empty())
    {
        App::console << "ERROR: No physics instance" << std::endl;
        Py_RETURN_NONE;
    }

    SDL_LockMutex(phyInstances_lock);

    bool fail;
    if (b == b)
        fail = !UpdatePhysBlend(&phyInstances.back(), filename, a, b);
    else if (a == a)
        fail = !UpdatePhysBlend(&phyInstances.back(), filename, 1.0f-a, a);
    else
        fail = !UpdatePhys(&phyInstances.back(), filename);

    if (fail)
    {
        // TODO: report error
	}

    SDL_UnlockMutex(phyInstances_lock);

    Py_RETURN_NONE;
}

static PyObject* EmbUpdatePhyFromString(PyObject *self, PyObject *args)
{
    char *string;

    if ( !PyArg_ParseTuple(args, "s:UpdatePhy2", &string) )
        return NULL;

    if (phyInstances.empty())
    {
        App::console << "ERROR: No physics instance" << std::endl;
        Py_RETURN_NONE;
    }

    SDL_LockMutex(phyInstances_lock);
    if ( !UpdatePhysFromString(&phyInstances.back(), string) )
    {
        // report error
	}
    SDL_UnlockMutex(phyInstances_lock);

    Py_RETURN_NONE;
}

static PyObject* EmbPollPhy(PyObject *self, PyObject *args)
{
    char *pollstring;

    if ( !PyArg_ParseTuple(args, "s:PollPhy", &pollstring) )
        return NULL;

    if (phyInstances.empty())
    {
        App::console << "ERROR: No physics instance" << std::endl;
        Py_RETURN_NONE;
    }
    SDL_LockMutex(phyInstances_lock);
    std::vector<REAL> ret = PollPhys(&phyInstances.back(), pollstring);
    SDL_UnlockMutex(phyInstances_lock);

    unsigned int size = ret.size();
    if ( size == 0 )
    {
        Py_RETURN_NONE;
    }
    else if ( size == 1 )
    {
        return Py_BuildValue("f", ret[0]);
    }

    PyObject *retlist = PyTuple_New( size );
    for (unsigned int i = 0; i < size; ++i)
    {
        PyObject *o = Py_BuildValue("f", ret[i]);
        Py_INCREF(o);
        PyTuple_SetItem(retlist, i, o);
    }
    return retlist;
}


static PyObject* EmbBind(PyObject *self, PyObject *args)
{
    char *event;
    PyObject *obj;

    //if ( !PyArg_ParseTuple(args, "sO!:bind", &event, &PyFunction_Type, &obj) )
    if ( !PyArg_ParseTuple(args, "sO:bind", &event, &obj) )
        return NULL;

    App::Bind(event, obj);

    Py_RETURN_NONE;
}
} // extern "C"

static PyMethodDef EmbMethods[] =
{
    {"ReloadVideo",    EmbReloadVideo,     METH_VARARGS, "Recreate window, reload shaders and textures"},
    {"LoadWorld",      EmbLoadWorld,       METH_VARARGS, "Load a world from a file"},
    {"SaveWorld",      EmbSaveWorld,       METH_VARARGS, "Save a world to a file"},
    {"ClearWorld",     EmbClearWorld,      METH_VARARGS, "Remove all objects and flatten the terrain"},
    {"Clear",          EmbClearConsole,    METH_VARARGS, "Remove all text in the console"},
    {"LoadPhy",        EmbLoadPhy,         METH_VARARGS, "Load a physics object from a file"},
    {"UpdatePhy",      EmbUpdatePhy,       METH_VARARGS, "Load a physics pose from a file"},
    {"UpdatePhy2",     EmbUpdatePhyFromString, METH_VARARGS, "Load a physics pose from a string"},
    {"PollPhy",        EmbPollPhy,         METH_VARARGS, "Poll info from a physics instance"},
    {"bind",           EmbBind,            METH_VARARGS, "Binds an event to an action (function or string). bind(\"mouse1_button1\", left_punch)"},
    {"unbind",         EmbUnBind,          METH_VARARGS, "Unbinds an event bound with \"bind\""},
    {"quit",           EmbQuit,            METH_VARARGS, "Quit the whole game"},
    {"write",          EmbWrite,           METH_VARARGS, "write function to override sys.stdout or sys.stderr"},
    {"MoveForward",    EmbPlayerMoveForward, METH_VARARGS, "walk forward"},
    {"MoveBack",       EmbPlayerMoveBack,  METH_VARARGS, "walk backwards"},
    {"MoveLeft",       EmbPlayerMoveLeft,  METH_VARARGS, "strafe left"},
    {"MoveRight",      EmbPlayerMoveRight, METH_VARARGS, "strafe right"},
    {"LookUp",         EmbPlayerLookUp,    METH_VARARGS, "look up"},
    {"LookDown",       EmbPlayerLookDown,  METH_VARARGS, "look down"},
    {"LookLeft",       EmbPlayerLookLeft,  METH_VARARGS, "look left"},
    {"LookRight",      EmbPlayerLookRight, METH_VARARGS, "look right"},
    {"Arm1Left",       EmbPlayerArm1Left,  METH_VARARGS, "App::m_player.MoveArm1(-value, 0);"},
    {"Arm1Right",      EmbPlayerArm1Right, METH_VARARGS, "App::m_player.MoveArm1(value, 0);"},
    {"Arm1Up",         EmbPlayerArm1Up,    METH_VARARGS, "App::m_player.MoveArm1(0, value);"},
    {"Arm1Down",       EmbPlayerArm1Down,  METH_VARARGS, "App::m_player.MoveArm1(0, -value);"},
    {"Arm2Left",       EmbPlayerArm2Left,  METH_VARARGS, "App::m_player.MoveArm2(-value, 0);"},
    {"Arm2Right",      EmbPlayerArm2Right, METH_VARARGS, "App::m_player.MoveArm2(value, 0);"},
    {"Arm2Up",         EmbPlayerArm2Up,    METH_VARARGS, "App::m_player.MoveArm2(0, value);"},
    {"Arm2Down",       EmbPlayerArm2Down,  METH_VARARGS, "App::m_player.MoveArm2(0, -value);"},
    {"Screenshot",     EmbScreenshot,      METH_VARARGS, "Save a screenshot to a .bmp image"},
    {"developer",      EmbDeveloper,       METH_VARARGS, "developer mode: non-zero gives verbose output"},
    {"wireframe",      EmbWireframe,       METH_VARARGS, "wireframe drawing. 1=front, 2=front and back, 0=off"},
    {"stereo3d",       EmbStereo3d,        METH_VARARGS, "stereoscopic 3d. 1=cross-eyed, 2=horizontally interlaced, 3=vertically interlaced, 0=off"},
    {"stereo3d_depth", EmbStereo3d_depth,  METH_VARARGS, "depth factor for stereo 3d"},
    {"stereo3d_focus", EmbStereo3d_focus,  METH_VARARGS, "the world distance to the screen, objects closer vill pop out"},
    {"view_depth",     EmbViewDepth,       METH_VARARGS, "how far things are rendered"},
    {NULL, NULL, 0, NULL}
};

bool InitPython()
{
    //assert( !Py_IsInitialized() );
    Py_Initialize();
    //PyImport_AddModule("Torped");
    Py_InitModule("Torped", EmbMethods);
    App::console << PyRun_SimpleString("import os");
    App::console << PyRun_SimpleString("import sys");
    App::console << PyRun_SimpleString("from Torped import *");
    App::console << PyRun_SimpleString("import Torped");
    App::console << PyRun_SimpleString("sys.stdout = Torped");
    App::console << PyRun_SimpleString("sys.stderr = Torped");
    App::console << PyRun_SimpleString("execfile('autoexec.py')");
    App::console << std::endl;

    //PyRun_SimpleString("bind()");
/*
    PyObject *modules = PySys_GetObject("modules");
    PyObject *main = PyDict_GetItemString(modules, "__main__");
    PyObject *dict = PyModule_GetDict(main);

    PyObject *ret = PyRun_String( "execfile('startup.py')", Py_file_input, dict, dict);

    if ( NULL == ret )
    {
        App::console << "jävla satans kukerror" << std::endl;
        //PyErr_Print();
    }
    else
    {
        App::console << "fungerar inte likt förbannat" << std::endl;
        Py_DECREF(ret);
    }
*/
    return true;
}

void QuitPython()
{
    //assert( Py_IsInitialized() );
    Py_Finalize();
}
