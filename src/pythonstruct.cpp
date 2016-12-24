#include <SDL/SDL_thread.h>
#include <string>
#include <Python.h>
#include "structmember.h"

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "gameapp.h"
#include "input.h"
#include "logging.h"
#include "menu.h"
#include "player.h"
#include "pythonstruct.h"

PyAction::PyAction(PyObject *a)
{
    action = a;
    Py_INCREF(action);
}

PyAction::~PyAction()
{
    Py_XDECREF(action);
}

void PyAction::Run(float value)
{
    if ( PyCallable_Check(action) )
    {
        PyObject *obj = PyObject_CallObject(action, Py_BuildValue("(f)", value));
        if (obj == NULL)
        {
            // complain
            return;
        }
        Py_DECREF(obj);
    }
    else if ( PyString_Check(action) && value != 0)
    {
        std::stringstream ss;
        ss << "value = " << value;
        PyRun_SimpleString( ss.str().c_str() );
        PyRun_SimpleString( PyString_AsString(action) );
    }
}

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

#define BEGIN_FUNC_FLOAT(name, funcname, description, body) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    float value; \
    if ( !PyArg_ParseTuple(args, "f:"#name, &value) ) \
        return NULL; \
    body \
}

#define BEGIN_FUNC_STRING(name, funcname, description, body) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    char *value; \
    if ( !PyArg_ParseTuple(args, "s:"#name, &value) ) \
        return NULL; \
    body \
}

#define BEGIN_FUNC_NOPARAM(name, funcname, description, body) \
static PyObject* funcname (PyObject *self, PyObject *args) \
{ \
    if ( !PyArg_ParseTuple(args, ":"#name) ) \
        return NULL; \
    body \
}

#define BEGIN_FUNC_CUSTOM(name, funcname, description) \
static PyObject* funcname (PyObject *self, PyObject *args)

CONSOLE_INT(App::developermode, "developer", EmbDeveloper)
CONSOLE_INT(App::wireframemode, "wireframe", EmbWireframe)
CONSOLE_INT(App::stereo3dmode, "stereo3d", EmbStereo3d)
CONSOLE_FLOAT(App::stereo3d_depth, "stereo3d_depth", EmbStereo3d_depth)
CONSOLE_FLOAT(App::stereo3d_focus, "stereo3d_focus", EmbStereo3d_focus)
CONSOLE_FLOAT(App::view_depth, "view_depth", EmbViewDepth)
CONSOLE_INT(App::profilermode, "profiler", EmbProfilerMode)


#define LIST_OF_FUNCTIONS \
BEGIN_FUNC_STRING("LoadWorld",   EmbLoadWorld, "Load a world from a file", {App::LoadWorld(value); Py_RETURN_NONE;}) \
BEGIN_FUNC_STRING("SaveWorld",   EmbSaveWorld, "Save a world to a file", {App::SaveWorld(value); Py_RETURN_NONE;}) \
BEGIN_FUNC_NOPARAM("ClearWorld", EmbClearWorld, "Remove all objects and flatten the terrain", {App::ClearWorld(); Py_RETURN_NONE;}) \
\
BEGIN_FUNC_NOPARAM("ReloadVideo", EmbReloadVideo, "Recreate window, reload shaders and textures", {App::ReloadVideo(); Py_RETURN_NONE;}) \
BEGIN_FUNC_STRING("Screenshot",   EmbScreenshot, "Save a screenshot to a .bmp image", {App::SaveScreenshot(value); Py_RETURN_NONE;}) \
\
BEGIN_FUNC_NOPARAM("Clear", EmbClearConsole, "Remove all text in the console", {App::ClearConsole(); Py_RETURN_NONE;}) \
BEGIN_FUNC_STRING("write",  EmbWrite, "write function to override sys.stdout or sys.stderr", {LOG_S(INFO) << value; Py_RETURN_NONE;}) \
BEGIN_FUNC_STRING("unbind", EmbUnBind, "Unbinds an event bound with \"bind\"", {App::UnBind(value); Py_RETURN_NONE;}) \
BEGIN_FUNC_NOPARAM("quit",  EmbQuit, "Quit the whole game", {App::Quit(); Py_RETURN_NONE;}) \
\
BEGIN_FUNC_FLOAT("MoveForward",EmbPlayerMoveForward, "walk forward", {App::player.vel.x = value; Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("MoveBack",   EmbPlayerMoveBack,    "walk backwards", {App::player.vel.x = -value; Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("MoveLeft",   EmbPlayerMoveLeft,    "strafe left", {App::player.vel.y = value; Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("MoveRight",  EmbPlayerMoveRight,   "strafe right", {App::player.vel.y = -value; Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("MoveUp",     EmbPlayerMoveUp,      "move up", {App::player.vel.z = value; Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("MoveDown",   EmbPlayerMoveDown,    "move down", {App::player.vel.z = -value; Py_RETURN_NONE;}) \
\
BEGIN_FUNC_FLOAT("LookUp",    EmbPlayerLookUp,    "look up", {App::player.MoveMouse(0, value); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("LookDown",  EmbPlayerLookDown,  "look down", {App::player.MoveMouse(0, -value); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("LookLeft",  EmbPlayerLookLeft,  "look left", {App::player.MoveMouse(-value, 0); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("LookRight", EmbPlayerLookRight, "look right", {App::player.MoveMouse(value, 0); Py_RETURN_NONE;}) \
\
BEGIN_FUNC_FLOAT("Arm1Left",  EmbPlayerArm1Left,  "", {App::player.MoveArm1(-value, 0); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("Arm1Right", EmbPlayerArm1Right, "", {App::player.MoveArm1(value, 0); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("Arm1Up",    EmbPlayerArm1Up,    "", {App::player.MoveArm1(0, value); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("Arm1Down",  EmbPlayerArm1Down,  "", {App::player.MoveArm1(0, -value); Py_RETURN_NONE;}) \
\
BEGIN_FUNC_FLOAT("Arm2Left",  EmbPlayerArm2Left,  "", {App::player.MoveArm2(-value, 0); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("Arm2Right", EmbPlayerArm2Right, "", {App::player.MoveArm2(value, 0); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("Arm2Up",    EmbPlayerArm2Up,    "", {App::player.MoveArm2(0, value); Py_RETURN_NONE;}) \
BEGIN_FUNC_FLOAT("Arm2Down",  EmbPlayerArm2Down,  "", {App::player.MoveArm2(0, -value); Py_RETURN_NONE;}) \

LIST_OF_FUNCTIONS

BEGIN_FUNC_STRING("Spawn", EmbSpawn, "",
{
    if ( !PhyInstance::InsertPhysXML(value) )
    {
        // report error
	}

    Py_RETURN_NONE;
})

BEGIN_FUNC_CUSTOM("Pose", EmbPose, "")
{
    char *name;
    float a,b;
    a = b = std::numeric_limits<float>::quiet_NaN();
    if ( !PyArg_ParseTuple(args, "s|ff:Pose", &name, &a, &b) )
        return NULL;

    if (phyInstances.empty())
    {
        LOG_S(ERROR) << "No physics instance";
        Py_RETURN_NONE;
    }

    std::lock_guard<std::mutex> lock(phyInstances_lock);

    bool fail;
    if (b == b)
        fail = !phyInstances.back().UpdatePhysBlend(name, a, b);
    else if (a == a)
        fail = !phyInstances.back().UpdatePhysBlend(name, 1.0f-a, a);
    else
        fail = !phyInstances.back().UpdatePhys(name);

    if (fail)
    {
        // TODO: report error
	}

    Py_RETURN_NONE;
}

BEGIN_FUNC_CUSTOM("PollPhy", EmbPollPhy, "")
{
    char *pollstring;

    if ( !PyArg_ParseTuple(args, "s:PollPhy", &pollstring) )
        return NULL;

    if (phyInstances.empty())
    {
        LOG_S(ERROR) << "No physics instance";
        Py_RETURN_NONE;
    }

    std::vector<REAL> ret;
    {
        std::lock_guard<std::mutex> lock(phyInstances_lock);
        ret = phyInstances.back().PollPhys(pollstring);
    }

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

BEGIN_FUNC_CUSTOM("Motor", EmbMotor, "")
{
    char *name;
    float x,y,z;
    x = y = z = std::numeric_limits<float>::quiet_NaN();
    if ( !PyArg_ParseTuple(args, "sfff:Motor", &name, &x, &y, &z) )
        return NULL;

    std::lock_guard<std::mutex> lock(phyInstances_lock);

    if (phyInstances.empty())
    {
        LOG_S(ERROR) << "No physics instance";
        Py_RETURN_NONE;
    }

    PhyInstance *inst = &phyInstances.back();
    TypeName tn;
    tn.type = "motor";
    tn.name = name;
    if ( inst->namesIndex.find(tn) != inst->namesIndex.end() )
        inst->phys->motors[inst->namesIndex[tn]].torque = Vec3r(x,y,z) * (inst->phys->time * inst->phys->time);

    Py_RETURN_NONE;
}

BEGIN_FUNC_CUSTOM("bind", EmbBind, "")
{
    char *event;
    PyObject *obj;

    //if ( !PyArg_ParseTuple(args, "sO!:bind", &event, &PyFunction_Type, &obj) )
    if ( !PyArg_ParseTuple(args, "sO:bind", &event, &obj) )
        return NULL;

    App::Bind(event, new PyAction(obj));

    Py_RETURN_NONE;
}
} // extern "C"

#undef BEGIN_FUNC_FLOAT
#undef BEGIN_FUNC_STRING
#undef BEGIN_FUNC_NOPARAM
#undef BEGIN_FUNC_CUSTOM

#define BEGIN_FUNC_FLOAT(name, funcname, description, body) \
{name, funcname, METH_VARARGS, description},

#define BEGIN_FUNC_STRING(name, funcname, description, body) \
{name, funcname, METH_VARARGS, description},

#define BEGIN_FUNC_NOPARAM(name, funcname, description, body) \
{name, funcname, METH_VARARGS, description},

#define BEGIN_FUNC_CUSTOM(name, funcname, description, body) \
{name, funcname, METH_VARARGS, description},


static PyMethodDef EmbMethods[] =
{
    {"Spawn",          EmbSpawn,           METH_VARARGS, "Load a physics object from a .xml file"},
    {"Pose",           EmbPose,            METH_VARARGS, "Activate a physics pose (blend with Pose(..., 0.25))"},
    {"PollPhy",        EmbPollPhy,         METH_VARARGS, "Poll info from a physics instance"},
    {"Motor",          EmbMotor,           METH_VARARGS, "set named motor torque: Motor(\"engine1\", 0, 0, 15)"},
    {"bind",           EmbBind,            METH_VARARGS, "Binds an event to an action (function or string). bind(\"mouse1_button1\", left_punch)"},
    {"developer",      EmbDeveloper,       METH_VARARGS, "developer mode: non-zero gives verbose output"},
    {"wireframe",      EmbWireframe,       METH_VARARGS, "wireframe drawing. 1=front, 2=front and back, 0=off"},
    {"stereo3d",       EmbStereo3d,        METH_VARARGS, "stereoscopic 3d. 1=cross-eyed, 2=horizontally interlaced, 3=vertically interlaced, 0=off"},
    {"stereo3d_depth", EmbStereo3d_depth,  METH_VARARGS, "depth factor for stereo 3d"},
    {"stereo3d_focus", EmbStereo3d_focus,  METH_VARARGS, "the world distance to the screen, objects closer vill pop out"},
    {"view_depth",     EmbViewDepth,       METH_VARARGS, "how far things are rendered"},
    {"profiler",       EmbProfilerMode,    METH_VARARGS, "show profiler timings"},
    LIST_OF_FUNCTIONS
    {NULL, NULL, 0, NULL}
};

typedef struct
{
    PyObject_HEAD
    int developer;
    int wireframe;
} convars_object;

static int
convars_set_int(convars_object *self, PyObject *value, void *closure)
{
    if (value == NULL)
    {
        PyErr_SetString(PyExc_TypeError, "Cannot delete convars");
        return -1;
    }

    if (! PyInt_Check(value))
    {
        PyErr_SetString(PyExc_TypeError, "Attribute value must be an int");
        return -1;
    }

    *reinterpret_cast<int*>( (char*)(self) + reinterpret_cast<ptrdiff_t>(closure) ) = PyInt_AsLong(value);

    return 0;
}

static PyObject *
convars_get_int(convars_object *self, void *closure)
{
    return  Py_BuildValue("i",
        *reinterpret_cast<int*>( (char*)(self) + reinterpret_cast<ptrdiff_t>(closure) )
        );
}

static PyMethodDef convars_methods[] =
{
    {NULL}  /* Sentinel */
};

static PyMemberDef convars_members[] =
{
    {"developer", T_INT, offsetof(convars_object, developer), 0,
     "developer mode"},
    {"wireframe", T_INT, offsetof(convars_object, wireframe), 0,
     "wireframe drawing. 1=front, 2=front and back, 0=off"},
    {NULL}  /* Sentinel */
};

static PyGetSetDef convars_getseters[] =
{
    {"developer",
     (getter)convars_get_int, (setter)convars_set_int,
     "developer mode",
     reinterpret_cast<void*>(int(offsetof(convars_object, developer)))},
    {"wireframe",
     (getter)convars_get_int, (setter)convars_set_int,
     "wireframe mode",
     reinterpret_cast<void*>(int(offsetof(convars_object, wireframe)))},
    {NULL}  /* Sentinel */
};

static PyTypeObject convars_type =
{
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    "Torped.convars",          // tp_name
    sizeof(convars_object),    // tp_basicsize
    0,                         // tp_itemsize
    0,                         // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    0,                         // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash
    0,                         // tp_call
    0,                         // tp_str
    0,                         // tp_getattro
    0,                         // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT,        // tp_flags
    "Console variables doc",   // tp_doc
    0,                         // tp_traverse
    0,                         // tp_clear
    0,                         // tp_richcompare
    0,                         // tp_weaklistoffset
    0,                         // tp_iter
    0,                         // tp_iternext
    convars_methods,           // tp_methods
    convars_members,           // tp_members
    0,                         // tp_getset
    0,                         // tp_base
    0,                         // tp_dict
    0,                         // tp_descr_get
    0,                         // tp_descr_set
    0,                         // tp_dictoffset
    0,                         // tp_init
    0,                         // tp_alloc
    PyType_GenericNew,         // tp_new
};

bool InitPython()
{
    //assert( !Py_IsInitialized() );
    Py_Initialize();

    if (PyType_Ready(&convars_type) < 0)
        return false;

    //PyImport_AddModule("Torped");
    PyObject *m = Py_InitModule("Torped", EmbMethods);

    Py_INCREF(&convars_type);
    PyModule_AddObject(m, "convars", (PyObject *)&convars_type);
/*
    PyObject *developer = Py_BuildValue("i", 0);
    PyModule_AddObject(m, "developer", developer);
*/
    PyRun_SimpleString("import os");
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("from Torped import *");
    PyRun_SimpleString("import Torped");
    //App::console << PyRun_SimpleString("import convars");
    PyRun_SimpleString("sys.stdout = Torped");
    PyRun_SimpleString("sys.stderr = Torped");
    PyRun_SimpleString("execfile('autoexec.py')");
    //App::console << std::endl;

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
