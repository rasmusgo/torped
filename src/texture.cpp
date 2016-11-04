#include <assert.h>
#include "physfsrwops.h"
#include <memory>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "texture.h"
#include "console.h"

const char* mygluErrorString(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR: No error has been recorded.";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored, and has no other side effect than to set the error flag.";
    case GL_INVALID_VALUE:
        return "A numeric argument is out of range. The offending command is ignored, and has no other side effect than to set the error flag.";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored, and has no other side effect than to set the error flag.";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW:This command would cause a stack overflow. The offending command is ignored, and has no other side effect than to set the error flag.";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW:This command would cause a stack underflow. The offending command is ignored, and has no other side effect than to set the error flag.";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
    }
    return "UNKNOWN ERROR";
}
#define gluErrorString mygluErrorString

#define LOG_IF_ERROR(msg) \
{ \
    GLenum err; \
    while ( (err = glGetError()) != GL_NO_ERROR ) \
    { \
        App::console << "GL ERROR " << msg << ": " \
            << gluErrorString(err) << std::endl; \
        App::FlushConsole(); \
    } \
}

static SDL_PixelFormat RGBA8Format =
{
	NULL, //SDL_Palette *palette;
	32, // Uint8  BitsPerPixel;
	8, //Uint8  BytesPerPixel;
	0, //Uint8  Rloss;
	0, //Uint8  Gloss;
	0, //Uint8  Bloss;
	0, //Uint8  Aloss;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	0,//Uint8  Rshift;
	8,//Uint8  Gshift;
	16,//Uint8  Bshift;
	24,//Uint8  Ashift;
	0x000000FF,//Uint32 Rmask;
	0x0000FF00,//Uint32 Gmask;
	0x00FF0000,//Uint32 Bmask;
	0xFF000000,//Uint32 Amask;
#else
	24,//Uint8  Rshift;
	16,//Uint8  Gshift;
	8,//Uint8  Bshift;
	0,//Uint8  Ashift;
	0xFF000000,//Uint32 Rmask;
	0x00FF0000,//Uint32 Gmask;
	0x0000FF00,//Uint32 Bmask;
	0x000000FF,//Uint32 Amask;
#endif
	/* RGB color key information */
	0,//Uint32 colorkey;
	/* Alpha value information (per-surface alpha) */
	0 //Uint8  alpha;
};

void FlipImageY(SDL_Surface *image)
{
    char *row = new char[image->pitch];
    int half_height = image->h/2;
    for (int i = 0, j=image->h-1; i < half_height; ++i, --j)
    {
        memcpy(row, (char*)image->pixels + i*image->pitch, image->pitch);
        memcpy((char*)image->pixels + i*image->pitch, (char*)image->pixels + j*image->pitch, image->pitch);
        memcpy((char*)image->pixels + j*image->pitch, row, image->pitch);
    }
    delete [] row;
    row = NULL;
}

// THE instance of Texture::textures (declared static in class)
typeof(Texture::textures) Texture::textures;
typeof(Texture::unique_textures) Texture::unique_textures;

Texture::Texture()
{
    unique = false;
    texturetype = GL_TEXTURE_2D;
}

Texture::~Texture()
{
    Release();
}

static SDL_Surface* LoadImage(const char *filename)
{
    SDL_RWops *rw = PHYSFSRWOPS_openRead(filename);

    if (rw == NULL)
    {
        return NULL;
    }

    return IMG_Load_RW(rw, 1);
}

void Texture::New(const char p_filename[])
{
    Release();

    assert(p_filename != NULL);

    unique = true;
    unique_textures.insert(this);
    id = 0;

    image = LoadImage(p_filename);

    if (!image)
    {
        App::console << "IMG_Load failed: " << SDL_GetError() << std::endl;
        return;
    }
    App::console << "Loading \"" << p_filename << "\": "
        << image->w << "x" << image->h << " "
        << int(image->format->BitsPerPixel) << " bpp" << std::endl;

    if ( image->w < 1 || image->w & (image->w - 1) ||
        image->h < 1 || image->h & (image->h - 1) )
    {
        App::console << "Couldn't load texture: Image size is not valid, must be power of 2" << std::endl;
        SDL_FreeSurface(image);
        image = NULL;
        return;
    }

    FlipImageY(image);

    if (image->format->BitsPerPixel != 32)
    {
        SDL_Surface *convert = SDL_ConvertSurface(image, &RGBA8Format, SDL_SWSURFACE);
        SDL_FreeSurface(image);
        image = convert;
    }

    glGenTextures(1, &id);
    LOG_IF_ERROR("glGenTextures");
    Update();
    LOG_IF_ERROR("Update");
}

void Texture::New(const int width, const int height)
{
    Release();

    unique = true;
    unique_textures.insert(this);
    id = 0;
    image = NULL;

    App::console << "Creating a new texture: "
        << width << "x" << height << " "
        << 32 << " bpp" << std::endl;

    if ( width < 1 || width & (width - 1) ||
        height < 1 || height & (height - 1) )
    {
        App::console << "Couldn't create texture: Image size is not valid, must be power of 2" << std::endl;
        return;
    }

    image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                 RGBA8Format.Rmask, RGBA8Format.Gmask,
                                 RGBA8Format.Bmask, RGBA8Format.Amask);

    if (!image)
    {
        App::console << "Couldn't create texture: " << SDL_GetError() << std::endl;
        return;
    }

    glGenTextures(1, &id);
    Update();
}

void Texture::Update()
{
    if (unique == false)
    {
        App::console << "WARNING: Texture::Update() was called on a non-unique texture";
        return;
    }

    if (image == NULL)
    {
        App::console << "WARNING: Texture::Update() image == NULL";
        return;
    }

    if (id == 0)
    {
        App::console << "WARNING: Texture::Update() id == 0";
        return;
    }

    glBindTexture(GL_TEXTURE_2D, id);
    LOG_IF_ERROR("glBindTexture");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    LOG_IF_ERROR("glPixelStorei");
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, image->w, image->h, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
    LOG_IF_ERROR("gluBuild2DMipmaps");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    LOG_IF_ERROR("glTexParameteri");

    // Disable texture repeat
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void Texture::Aquire(const char p_filename[])
{
    if (p_filename == NULL)
        p_filename = "null.png";

    Release();

    unique = false;
    filename = std::string(p_filename);

    if (textures.find(filename) == textures.end())
        LoadTexture(filename.c_str());

    if (textures.find(filename) == textures.end())
    {
    	App::console << "Texture::Aquire failed for \"" << filename << "\"." << std::endl;
    	return;
    }
    //m_texture = textures[filename].id;
    textures[filename].ref++;
    //App::console << "Texture::Aquire succeded" << std::endl;
}

void Texture::Release()
{
    if (unique)
    {
        if (image)
        {
            SDL_FreeSurface(image);
            image = NULL;
        }
        glDeleteTextures(1 , &id);
        unique_textures.erase(this);
        id = 0;
    }
    else
    {
        typeof(textures.begin()) it = textures.find(filename);

        if (it != textures.end())
        {
            if (--it->second.ref == 0)
            {
                glDeleteTextures(1 , &it->second.id);
                textures.erase(it);
            }
        }
        //m_texture = 0;
        filename = "";
    }
}

void Texture::LoadTexture(const char *p_filename)
{
    SDL_Surface *image = LoadImage(p_filename);
    if (!image)
    {
        App::console << "IMG_Load failed: " << SDL_GetError() << std::endl;
        return;
    }
    App::console << "Loading \"" << p_filename << "\": "
        << image->w << "x" << image->h << " "
        << int(image->format->BitsPerPixel) << " bpp" << std::endl;

    if ( image->w < 1 || image->w & (image->w - 1) ||
        image->h < 1 || image->h & (image->h - 1) )
    {
        LoadCubeMap(p_filename, image);
        SDL_FreeSurface(image);
        image = NULL;
        return;
    }

    FlipImageY(image);
    IdRef id_ref={0, 0, GL_TEXTURE_2D};
    glGenTextures(1, &id_ref.id);
    glBindTexture(GL_TEXTURE_2D, id_ref.id);

    switch (image->format->BitsPerPixel)
    {
    case 32:
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image->w, image->h, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
        break;
    case 24:
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, image->w, image->h, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
        break;
    case 16:
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image->w, image->h, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1, image->pixels);
        break;
    case 8:
    {
        SDL_Surface *convert = SDL_ConvertSurface(image, &RGBA8Format, SDL_SWSURFACE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, convert->w, convert->h, GL_RGBA, GL_UNSIGNED_BYTE, convert->pixels);

        SDL_FreeSurface(convert);
        break;
    }
    default:
        App::console << image->format->BitsPerPixel << " bpp unhandled" << std::endl;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    LOG_IF_ERROR("glTexParameteri");

    SDL_FreeSurface(image);
    textures[p_filename] = id_ref;
}

void Texture::LoadCubeMap(const char *p_filename, SDL_Surface *image)
{
    if (!image)
    {
        App::console << "LoadCubeMap failed: image == NULL" << std::endl;
        return;
    }

    int width = image->w / 3, height = image->h / 2;

    if ( image->w % 3 ||
         width < 1 || width & (width - 1) ||
         height != width )
    {
        App::console << "Couldn't load texture: Image size is not valid, must be power of 2" << std::endl;
        return;
    }

    LOG_IF_ERROR("pre glGenTextures");

    FlipImageY(image);
    IdRef id_ref={0, 0, GL_TEXTURE_CUBE_MAP};
    glGenTextures(1, &id_ref.id);
    LOG_IF_ERROR("glGenTextures");
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_ref.id);
    LOG_IF_ERROR("glBindTexture");

    SDL_Surface *tmp = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 32,
                                             RGBA8Format.Rmask, RGBA8Format.Gmask,
                                             RGBA8Format.Bmask, RGBA8Format.Amask );
    if (tmp == NULL)
    {
        App::console << "SDL_CreateRGBSurface failed: " << SDL_GetError() << std::endl;
        App::FlushConsole();
        return;
    }

    SDL_Rect srcrect = {0, height, width, height};
    SDL_Rect dstrect = {0, 0, width, height};

    SDL_BlitSurface(image, &srcrect, tmp, &dstrect);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
    LOG_IF_ERROR("glTexSubImage2D 1");

    srcrect.x += width;
    dstrect.x = 0; dstrect.y = 0;
    SDL_BlitSurface(image, &srcrect, tmp, &dstrect);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
    LOG_IF_ERROR("glTexSubImage2D 2");

    srcrect.x += width;
    dstrect.x = 0; dstrect.y = 0;
    SDL_BlitSurface(image, &srcrect, tmp, &dstrect);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
    LOG_IF_ERROR("glTexSubImage2D 3");

    srcrect.x = 0;
    srcrect.y = 0;

    dstrect.x = 0; dstrect.y = 0;
    SDL_BlitSurface(image, &srcrect, tmp, &dstrect);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
    LOG_IF_ERROR("glTexSubImage2D 4");

    srcrect.x += width;
    dstrect.x = 0; dstrect.y = 0;
    SDL_BlitSurface(image, &srcrect, tmp, &dstrect);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
    LOG_IF_ERROR("glTexSubImage2D 5");

    srcrect.x += width;
    dstrect.x = 0; dstrect.y = 0;
    SDL_BlitSurface(image, &srcrect, tmp, &dstrect);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
    LOG_IF_ERROR("glTexSubImage2D 6");

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    LOG_IF_ERROR("glTexParameteri");

    SDL_FreeSurface(tmp);
    tmp = NULL;

    textures[p_filename] = id_ref;
}

// problem: texture instances doesn't pick up changes in reference ids
void Texture::ReloadAll()
{
    typeof(textures.begin()) it;
    typeof(unique_textures.begin()) it2;
    GLuint ref;

    for (it = textures.begin(); it != textures.end(); ++it)
    {
        if ( glIsTexture(it->second.id) )
            glDeleteTextures(1 , &it->second.id);
    }

    for (it2 = unique_textures.begin(); it2 != unique_textures.end(); ++it2)
    {
        if ( glIsTexture((*it2)->id) )
            glDeleteTextures(1 , &(*it2)->id);
    }

    for (it = textures.begin(); it != textures.end(); ++it)
    {
    	ref = it->second.ref;
    	LoadTexture(it->first.c_str());
    	it->second.ref = ref;
    }

    for (it2 = unique_textures.begin(); it2 != unique_textures.end(); ++it2)
    {
        glGenTextures(1, &(*it2)->id);
        (*it2)->Update();
    }

    for (it = textures.begin(); it != textures.end(); ++it)
    {
        App::console << "name: \"" << it->first.c_str() << "\" id: " << it->second.id << std::endl;
    }
}

void Texture::Bind()
{
    if (unique)
    {
        glBindTexture(texturetype, id);
        return;
    }

    // iterator
    typeof(textures.begin()) i = textures.find(filename);

    if (i == textures.end())
    {
        glBindTexture(texturetype, 0);
        return;
    }

    glBindTexture(i->second.type, i->second.id);
}

void Texture::Enable()
{
    if (unique)
    {
        glEnable(texturetype);
        return;
    }

    // iterator
    typeof(textures.begin()) i = textures.find(filename);

    if (i == textures.end())
    {
        glEnable(GL_TEXTURE_2D);
        return;
    }

    glEnable(i->second.type);
}

void Texture::Disable()
{
    if (unique)
    {
        glDisable(texturetype);
        return;
    }

    // iterator
    typeof(textures.begin()) i = textures.find(filename);

    if (i == textures.end())
    {
        glDisable(GL_TEXTURE_2D);
        return;
    }

    glDisable(i->second.type);
}

GLuint Texture::GetID()
{
    // iterator
    typeof(textures.begin()) i = textures.find(filename);

    if (i == textures.end())
        return 0;
    return i->second.id;
}
