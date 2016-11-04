#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

void LuaReportErrors(lua_State *L, int status);
void LuaRunFile(lua_State *L, const char *filename);
void LuaWrapConsoleCmds(lua_State* L);
int LuaCmdClear(lua_State* L);
int LuaCmdLoad(lua_State* L);
int LuaCmdSave(lua_State* L);
int LuaCmdSpawn(lua_State* L);
int LuaCmdReloadVideo(lua_State* L);
int LuaCmdScreenshot(lua_State* L);
int LuaCmdBind(lua_State* L);
int LuaCmdUnBind(lua_State* L);
int LuaCmdQuit(lua_State* L);
int LuaCmdPlayer(lua_State* L);
int LuaCmdPrint(lua_State* L);
int LuaCmdExec(lua_State* L);

/*
{"developer",      EmbDeveloper,       METH_VARARGS, "developer mode: non-zero gives verbose output"},
{"wireframe",      EmbWireframe,       METH_VARARGS, "wireframe drawing. 1=front, 2=front and back, 0=off"},
{"stereo3d",       EmbStereo3d,        METH_VARARGS, "stereoscopic 3d. 1=cross-eyed, 2=horizontally interlaced, 3=vertically interlaced, 0=off"},
{"stereo3d_depth", EmbStereo3d_depth,  METH_VARARGS, "depth factor for stereo 3d"},
{"stereo3d_focus", EmbStereo3d_focus,  METH_VARARGS, "the world distance to the screen, objects closer vill pop out"},
{"view_depth",     EmbViewDepth,       METH_VARARGS, "how far things are rendered"},
{"profiler",       EmbProfilerMode,    METH_VARARGS, "show profiler timings"},

BEGIN_FUNC_NOPARAM("Clear", EmbClearConsole, "Remove all text in the console", {App::ClearConsole(); Py_RETURN_NONE;}) \
BEGIN_FUNC_STRING("write",  EmbWrite, "write function to override sys.stdout or sys.stderr", {App::console << value << std::flush; Py_RETURN_NONE;}) \

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

    {"Spawn",          EmbSpawn,           METH_VARARGS, "Load a physics object from a .xml file"},
    {"Pose",           EmbPose,            METH_VARARGS, "Activate a physics pose (blend with Pose(..., 0.25))"},
    {"PollPhy",        EmbPollPhy,         METH_VARARGS, "Poll info from a physics instance"},
    {"Motor",          EmbMotor,           METH_VARARGS, "set named motor torque: Motor(\"engine1\", 0, 0, 15)"},
    {"bind",           EmbBind,            METH_VARARGS, "Binds an event to an action (function or string). bind(\"mouse1_button1\", left_punch)"},

*/
