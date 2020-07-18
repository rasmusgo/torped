#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include "SDL_image.h"
//#include "SDL_opengl.h"
#include <string>
#include <map>
#include <set>

void FlipImageY(SDL_Surface *image);

class Texture
{
public:
    Texture();
    ~Texture();
    void Acquire(const char p_filename[]);
    void New(const char p_filename[]);
    void New(const int width, const int height);
    void Update();
    void Release();
    void Bind();
    void Enable();
    void Disable();
    GLuint GetID();
    static void ReloadAll();
private:
    static void LoadTexture(const char *p_filename);
    static void LoadCubeMap(const char *p_filename, SDL_Surface *image);

    bool unique;

    // only used if texture is unique (like a decal map)
    SDL_Surface *image;
    GLuint id;
    GLuint texturetype;

    // not used if texture is unique
    std::string filename;

    struct IdRef
    {
    	GLuint id; // opengl texture id
    	GLuint ref; // reference count
        GLuint type;
    };
    static std::map<std::string, IdRef> textures;
    static std::set<Texture*> unique_textures;
};
