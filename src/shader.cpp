#include <gl/glew.h>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "shader.h"
#include "console.h"
#include "physfsstruct.h"

#define LOG_IF_ERROR(msg) \
{ \
    GLenum err; \
    while ( (err = glGetError()) != GL_NO_ERROR ) \
    { \
        App::console <<  __FILE__ << ":" << __LINE__ << " in " << __FUNCTION__ << ": " << msg << ": GL ERROR: " \
            << gluErrorString(err) << std::endl; \
        App::FlushConsole(); \
    } \
}

// Definition of Shader static variables
typeof(Shader::vertex_shaders) Shader::vertex_shaders;
typeof(Shader::fragment_shaders) Shader::fragment_shaders;
typeof(Shader::programs) Shader::programs;

Shader::Shader()
{

}

Shader::~Shader()
{
    Release();
}

void Shader::Aquire(const std::string &vertex_file, const std::string &fragment_file)
{
    Release();

    program.vertex_name = vertex_file;
    program.fragment_name = fragment_file;

    // Load if not loaded
    if ( programs.find(program) == programs.end() )
    {
        LoadProgram(program);

        // Fail if load failed
        if ( programs.find(program) == programs.end() )
        {
            program.vertex_name = "";
            program.fragment_name = "";
            return;
        }
    }

    // Add reference
    programs[program].ref++;
}

void Shader::Release()
{
    typeof(programs.begin()) it = programs.find(program);

    if ( it != programs.end() )
    {
        typeof(vertex_shaders.end()) vert = vertex_shaders.find(program.vertex_name);
        typeof(fragment_shaders.end()) frag = fragment_shaders.find(program.fragment_name);

        if ( vert != vertex_shaders.end() )
        {
            if ( --vert->second.ref == 0 )
            {
                glDeleteShader(vert->second.id);
                vertex_shaders.erase(vert);
            }
        }

        if ( frag != fragment_shaders.end() )
        {
            if ( --frag->second.ref == 0 )
            {
                glDeleteShader(frag->second.id);
                fragment_shaders.erase(frag);
            }
        }

        if ( --it->second.ref == 0)
        {
            glDeleteProgram(it->second.id);
            programs.erase(it);
        }
    }

    program.vertex_name = "";
    program.fragment_name = "";
}

void Shader::Enable()
{
    typeof(programs.begin()) it = programs.find(program);

    if (it != programs.end())
        glUseProgramObjectARB(it->second.id);
    else
        glUseProgramObjectARB(0);
}

void Shader::Disable()
{
    glUseProgramObjectARB(0);
}

void Shader::ReloadAll()
{
    LOG_IF_ERROR("");

    for (typeof(vertex_shaders.end()) vert = vertex_shaders.begin(); vert != vertex_shaders.end(); ++vert)
    {
        if (glIsShader(vert->second.id))
            glDeleteShader(vert->second.id);
    }
    vertex_shaders.clear();

    LOG_IF_ERROR("");

    for (typeof(fragment_shaders.end()) frag = fragment_shaders.begin(); frag != fragment_shaders.end(); ++frag)
    {
        if (glIsShader(frag->second.id))
            glDeleteShader(frag->second.id);
    }
    fragment_shaders.clear();

    LOG_IF_ERROR("");

    for (typeof(programs.end()) prog = programs.begin(); prog != programs.end(); ++prog)
    {
        if (glIsProgram(prog->second.id))
            glDeleteProgram(prog->second.id);
    }

    LOG_IF_ERROR("");

    for (typeof(programs.end()) prog = programs.begin(); prog != programs.end(); ++prog)
    {
        LoadProgram(prog->first);
    }

    LOG_IF_ERROR("");
}

void Shader::LoadShader(const GLuint type, const std::string &filename)
{
    if (filename == "" || !(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER))
        return;

    LOG("Loading shader \"" << filename << "\"...");

    GLchar *buffer = NULL;
    GLint length = PhysFSLoadFile(filename.c_str(), buffer);

    if (length == 0)
    {
        LOG("Failed to open \"" << filename << "\".");
        return;
    }

    IdRef id_ref = {0, 0};
    id_ref.id = glCreateShader(type);
    glShaderSource(id_ref.id, 1, (const GLchar**)&buffer, &length);

    delete [] buffer;
    buffer = NULL;

    GLint status;
    glCompileShader(id_ref.id);
    glGetObjectParameterivARB(id_ref.id, GL_COMPILE_STATUS, &status);

    if (!status)
    {
        // reusing char *buffer and unsigned int length
        glGetShaderiv(id_ref.id, GL_INFO_LOG_LENGTH , &length);

        if (length > 1)
        {
            buffer = new char[length];

            glGetInfoLogARB(id_ref.id, length, NULL, buffer);
            LOG("Failed to compile shader \"" << filename << "\":\n" << buffer);

            delete [] buffer;
            buffer = NULL;
        }
        else
            LOG("Failed to compile shader \"" << filename << "\": Unknown error.");

        glDeleteShader(id_ref.id);
        return;
    }

    if (type == GL_VERTEX_SHADER)
        vertex_shaders[filename] = id_ref;
    else
        fragment_shaders[filename] = id_ref;

}

void Shader::LoadProgram(const ProgramName &prog)
{
    LOG_IF_ERROR("LoadProgram start");

    typeof(vertex_shaders.end()) vert = vertex_shaders.find(prog.vertex_name);
    typeof(fragment_shaders.end()) frag = fragment_shaders.find(prog.fragment_name);

    if ( vert == vertex_shaders.end() )
    {
        LoadShader(GL_VERTEX_SHADER, prog.vertex_name);
        vert = vertex_shaders.find(prog.vertex_name);
    }

    if ( frag == fragment_shaders.end() )
    {
        LoadShader(GL_FRAGMENT_SHADER, prog.fragment_name);
        frag = fragment_shaders.find(prog.fragment_name);
    }

    LOG_IF_ERROR("LoadShaders");

    IdRef id_ref = {0, 0};
    id_ref.id = glCreateProgram();

    if ( vert != vertex_shaders.end() )
    {
        glAttachShader(id_ref.id, vert->second.id);
        vert->second.ref++;
    }

    if ( frag != fragment_shaders.end() )
    {
        glAttachShader(id_ref.id, frag->second.id);
        frag->second.ref++;
    }

    LOG_IF_ERROR("AttachShaders");

    GLint status;
    glLinkProgram(id_ref.id);
    LOG_IF_ERROR("LinkProgram");

    glGetObjectParameterivARB(id_ref.id, GL_OBJECT_LINK_STATUS_ARB, &status);
    LOG_IF_ERROR("Get compile status");

    if (!status)
    {
        GLchar *buffer = NULL;
        GLint length = 0;
        glGetShaderiv(id_ref.id, GL_INFO_LOG_LENGTH , &length);

        if (length > 1)
        {
            buffer = new char[length];

            glGetInfoLogARB(id_ref.id, length, NULL, buffer);
            LOG("Failed to link program:\n" << buffer);

            delete [] buffer;
            buffer = NULL;
        }
        else
            LOG("Failed to link program: Unknown error.");

        glDeleteProgram(id_ref.id);
        return;
    }

    programs[prog] = id_ref;
    LOG_IF_ERROR("End of Shader::LoadProgram(..)");
}
