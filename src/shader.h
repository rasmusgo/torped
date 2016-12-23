#ifndef SHADER_H
#define SHADER_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <GL/glew.h>
#include <string>
#include <map>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

class Shader
{
public:
    Shader();
    ~Shader();
    void Aquire(const std::string &vertex_file, const std::string &fragment_file);
    void Release();
    void Enable();
    void Disable();
    static void ReloadAll();

private:

    struct ProgramName
    {
        std::string vertex_name;
        std::string fragment_name;
        bool operator < (const ProgramName &a) const
        {
            if (vertex_name < a.vertex_name)
                return true;

            if (vertex_name != a.vertex_name)
                return false;

            if (fragment_name < a.fragment_name)
                return true;

            return false;
        }
    } program;

    static void LoadShader(const GLuint type, const std::string &filename);
    static void LoadProgram(const ProgramName &prog);

    struct IdRef
    {
    	GLuint id; // opengl id
    	GLuint ref; // reference count
    };

    static std::map<std::string, IdRef> vertex_shaders;
    static std::map<std::string, IdRef> fragment_shaders;
    static std::map<ProgramName, IdRef> programs;
};

#endif // SHADER_H
