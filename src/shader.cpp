#include <GL/glew.h>

#include "logging.hpp"
#include "physfsstruct.hpp"
#include "shader.hpp"

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
    auto program_it = programs.find(program);

    if ( program_it != programs.end() )
    {
        auto vert_it = vertex_shaders.find(program.vertex_name);
        auto frag_it = fragment_shaders.find(program.fragment_name);

        if ( vert_it != vertex_shaders.end() )
        {
            if ( --vert_it->second.ref == 0 )
            {
                glDeleteShader(vert_it->second.id);
                vertex_shaders.erase(vert_it);
            }
        }

        if ( frag_it != fragment_shaders.end() )
        {
            if ( --frag_it->second.ref == 0 )
            {
                glDeleteShader(frag_it->second.id);
                fragment_shaders.erase(frag_it);
            }
        }

        if ( --program_it->second.ref == 0)
        {
            glDeleteProgram(program_it->second.id);
            programs.erase(program_it);
        }
    }

    program.vertex_name = "";
    program.fragment_name = "";
}

void Shader::Enable()
{
    auto program_it = programs.find(program);

    if (program_it != programs.end())
        glUseProgramObjectARB(program_it->second.id);
    else
        glUseProgramObjectARB(0);
}

void Shader::Disable()
{
    glUseProgramObjectARB(0);
}

void Shader::SetUniform(const std::string &name, float x, float y, float z, float w)
{
    LOG_IF_ERROR("");
    auto program_it = programs.find(program);
    if (program_it != programs.end())
    {
        const GLuint program_ref = program_it->second.id;
        const GLint uniform_location = glGetUniformLocation(program_ref, name.c_str());
        LOG_IF_ERROR("");
        if (uniform_location != -1)
        {
            glUniform4f(uniform_location, x, y, z, w);
            //glProgramUniform4f(program_ref, uniform_location, x, y, z, w);
            LOG_IF_ERROR("");
        }
        else
        {
            LOG_F(WARNING, "Could not set uniform '%s' for program '%s' + '%s': glGetUniformLocation failed.",
                name.c_str(), program.vertex_name.c_str(), program.fragment_name.c_str());
        }
    }
    else
    {
        LOG_F(WARNING, "Could not set uniform '%s', program '%s' + '%s' is invalid.",
            name.c_str(), program.vertex_name.c_str(), program.fragment_name.c_str());
    }
}

void Shader::ReloadAll()
{
    LOG_IF_ERROR("");

    for (auto& vert : vertex_shaders)
    {
        if (glIsShader(vert.second.id))
            glDeleteShader(vert.second.id);
    }
    vertex_shaders.clear();

    LOG_IF_ERROR("");

    for (auto& frag : fragment_shaders)
    {
        if (glIsShader(frag.second.id))
            glDeleteShader(frag.second.id);
    }
    fragment_shaders.clear();

    LOG_IF_ERROR("");

    for (auto& prog : programs)
    {
        if (glIsProgram(prog.second.id))
            glDeleteProgram(prog.second.id);
    }

    LOG_IF_ERROR("");

    for (auto& prog : programs)
    {
        LoadProgram(prog.first);
    }

    LOG_IF_ERROR("");
}

void Shader::LoadShader(const GLuint type, const std::string &filename)
{
    if (filename == "" || !(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER))
        return;

    LOG_S(INFO) << "Loading shader \"" << filename << "\"...";

    GLchar *buffer = NULL;
    GLint length = PhysFSLoadFile(filename.c_str(), buffer);

    if (length == 0)
    {
        LOG_S(ERROR) << "Failed to open \"" << filename << "\".";
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
            LOG_S(ERROR) << "Failed to compile shader \"" << filename << "\":\n" << buffer;

            delete [] buffer;
            buffer = NULL;
        }
        else
        {
            LOG_S(ERROR) << "Failed to compile shader \"" << filename << "\": Unknown error.";
        }
        glDeleteShader(id_ref.id);
        id_ref.id = 0;
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
            LOG_S(ERROR) << "Failed to link program:\n" << buffer;

            delete [] buffer;
            buffer = NULL;
        }
        else
        {
            LOG_S(ERROR) << "Failed to link program: Unknown error.";
        }

        glDeleteProgram(id_ref.id);
        id_ref.id = 0;
        return;
    }

    programs[prog] = id_ref;
    LOG_IF_ERROR("End of Shader::LoadProgram(..)");
}
