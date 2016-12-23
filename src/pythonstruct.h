#ifndef PYTHONSTRUCT_H
#define PYTHONSTRUCT_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "action.h"

class PyAction: public Action
{
public:
    PyAction(PyObject *a);
    virtual ~PyAction();
    virtual void Run(float value);

private:
    PyObject *action;
};

bool InitPython();
void QuitPython();

#endif //PYTHONSTRUCT_H
