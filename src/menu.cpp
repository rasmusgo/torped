#include <deque>
#include <mutex>

#include "SDL_getenv.h"
#include <angelscript.h>
#include <GL/glew.h>
#ifdef __APPLE__
#include <glut.h>
#else
#include <GL/glut.h>
#endif
#include <SDL.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

#include "comparelist.h"
#include "console.h"
#include "gameapp.h"
#include "input.h"
#include "luawrap.h"
#include "menu.h"
#include "shader.h"

namespace App
{
    //std::stringstream console;
    Console console;
    std::deque<std::string> consoleLines;
    std::vector<std::string> commands;
    Texture consoleBackground;
    Texture angelscriptIcon;
    Texture luaIcon;

    inline void DrawString(void *font, const char string[])
    {
        int len = strlen(string);
        for (int i=0; i<len; i++)
            glutBitmapCharacter(font, (unsigned char)string[i]);
    }

    void SetupMenu()
    {
        if (consoleBackground.GetID() == 0)
            consoleBackground.Aquire("consoleback.png");

        if (angelscriptIcon.GetID() == 0)
            angelscriptIcon.Aquire("icon_angelscript.png");

        if (luaIcon.GetID() == 0)
            luaIcon.Aquire("icon_lua.png");

        glViewport(0, 0, xRes, yRes);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glLineWidth(1);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
        //glDisable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, xRes, 0, yRes);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        SDL_WM_GrabInput(SDL_GRAB_OFF);
        SDL_ShowCursor(SDL_ENABLE);

        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
        SDL_EnableUNICODE(1);
        DrawMenu();
    }

    void DrawMenu()
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        consoleBackground.Enable();
        consoleBackground.Bind();
        glColor3f(1, 1, 1);

        glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex2f(0,0);
        glTexCoord2f( 1,0);
        glVertex2f( xRes,0);
        glTexCoord2f( 1, 1);
        glVertex2f( xRes, yRes);
        glTexCoord2f(0, 1);
        glVertex2f(0, yRes);
        glEnd();

        consoleBackground.Disable();

        luaIcon.Enable();
        luaIcon.Bind();

        glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex2f(0,0);
        glTexCoord2f( 1,0);
        glVertex2f( 15,0);
        glTexCoord2f( 1, 1);
        glVertex2f( 15, 15);
        glTexCoord2f(0, 1);
        glVertex2f(0, 15);
        glEnd();

        luaIcon.Disable();

        glColor3f(0.8, 0.8, 0.8);
        glRasterPos2f( 18, 5 );
        DrawString( GLUT_BITMAP_9_BY_15, ( commands.back()).c_str() );
        //for (int i = 'a'; i< 256; ++i)
        //    DrawString( GLUT_BITMAP_9_BY_15, (char*)&i );

        int height = 15; //glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
        for ( int i = console.lines.size() - 1, y = 20; y < yRes && i >= 0; y += height, --i )
        {
            glRasterPos2f( 0, y );
            DrawString( GLUT_BITMAP_9_BY_15, console.lines[i].message.c_str() );
        }
        //DrawString(GLUT_BITMAP_HELVETICA_18, "Torped");
        //glRasterPos2f( -0.5, 0.5);
        //glMatrixMode(GL_MODELVIEW);
        SDL_GL_SwapBuffers();
    }

    void FlushConsole()
    {
        /*
        std::string line;

        while ( std::getline(App::console, line,'\n') )
        {
            consoleLines.push_back(line);
            while (consoleLines.size() > 64)
                consoleLines.pop_front();
            std::cout << line << std::endl;
        }

        // remove end-of-file flag
        App::console.clear();
        */
    }

    void ClearConsole()
    {
        App::console.clear();
    }

    int MenuFrame()
    {
        bool redraw = false;
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_VIDEOEXPOSE:
                redraw = true;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    SwitchMode(GAME);
                    return 1;
                }
                if ( event.key.keysym.sym == SDLK_RETURN && event.key.keysym.mod & KMOD_ALT )
                {
                    fullscreen = ! fullscreen;
                    ReloadVideo();
                    return 1;
                }
                if ((event.key.keysym.sym == SDLK_F4 && event.key.keysym.mod & KMOD_ALT) ||
                    (event.key.keysym.sym == SDLK_q && event.key.keysym.mod & KMOD_CTRL))
                {
                    return 0; // quits
                }
                if ( event.key.keysym.sym == SDLK_BACKSPACE )
                {
                    int len = commands.back().length();
                    if ( len > 0 )
                    {
                        commands.back().erase(len-1, 1);
                        redraw = true;
                    }
                    break;
                }

                // FIXME: fetch_pos should be moved because it will not reset
                // when the engine is restarted
                static int fetch_pos;

                if ( event.key.keysym.sym == SDLK_RETURN )
                {
                    LOG_S(INFO) << "> " << commands.back();
                    redraw = true;

                    fetch_pos = commands.size();
                    commands.push_back("");
                    break;
                }

                if ( event.key.keysym.sym == SDLK_UP )
                {
                    int size = commands.size();
                    --fetch_pos;
                    if (fetch_pos < 0)
                        fetch_pos = 0;
                    if (fetch_pos >= size-1)
                    {
                        fetch_pos = size-1;
                        commands.back() = "";
                    } else

                    if ( size >= 2 )
                    {
                        commands.back() = commands[fetch_pos];
                    }
                    redraw = true;
                    break;
                }

                if ( event.key.keysym.sym == SDLK_DOWN )
                {
                    int size = commands.size();
                    ++fetch_pos;
                    if (fetch_pos < 0)
                        fetch_pos = 0;
                    if (fetch_pos >= size-1)
                    {
                        fetch_pos = size-1;
                        commands.back() = "";
                    } else

                    if ( size >= 2 )
                    {
                        commands.back() = commands[fetch_pos];
                    }
                    redraw = true;
                    break;
                }

                if ( event.key.keysym.unicode > 0 && event.key.keysym.unicode < 256 )
                {
                    commands.back() += static_cast<char>(event.key.keysym.unicode);
                    redraw = true;
                    break;
                }
                break;
            case SDL_KEYUP:
                break;
            case SDL_QUIT:
                return 0;
            default:
                break;
            }
        }

        if ( !App::console.empty() )
        {
            redraw = true;
        }
        else
        {
            // remove end-of-file flag
            App::console.clear();
        }


        if ( redraw )
        {
            DrawMenu();
            redraw = false;
        }
        FlushMice();
        SDL_Delay(10); // Eat less CPU
        return 1;
    }

} // namespace App
