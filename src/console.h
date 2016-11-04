#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <deque>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif

namespace App
{
    //extern std::stringstream console;
    extern int developermode;
    void FlushConsole();

    class Console: public std::stringstream
    {
    public:
    /*
        Console()
        {
            std::ostream::rdbuf(std::cout.rdbuf());
        }
    */
        template<class T> friend Console& operator << (Console &con, const T &a)
        {
            std::cout << a;

            std::ostream buffer(con.rdbuf());
            buffer << a;

            return con;
        }

        template<typename _CharT, typename _Traits>
        friend Console&
        //basic_ostream<_CharT, _Traits>::
        operator<<(Console &con, std::basic_ios<_CharT, _Traits>& (*__pf)(std::basic_ios<_CharT, _Traits>&))
        {
            con << "FITTA!!";
            __pf(std::cout);
            __pf(con);
            return con;
            //__pf(*this);
            //return *this;
        }

    /*
        template<class T> Console& put(T __c)
        {
            std::cout.put(__c);
            std::ostream buffer(rdbuf());
            buffer.put(__c);
            return *this;
        }
    */
        void Clear()
        {
            lines.clear();
        }

        void pump()
        {
            std::istream buffer(rdbuf());
            std::string line;
            while ( std::getline(buffer, line,'\n') )
            {
                lines.push_back(line);
                while (lines.size() > 64)
                    lines.pop_front();
            }
            //buffer.clear();
        }

    private:
        std::deque<std::string> lines;
        friend void DrawMenu();
    };

    extern Console console;
}

#define LOG(msg) \
{ \
    App::console << msg << std::endl; \
    App::FlushConsole(); \
}

#endif // CONSOLE_H
