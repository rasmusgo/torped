#pragma once

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

#include "logging.h"

namespace App
{
    //extern std::stringstream console;
    extern int developermode;
    void FlushConsole();

    class Console
    {
    public:
        static void LogHandler(void* user_data, const loguru::Message& message)
        {
            CHECK_NOTNULL_F(user_data);
            Console* console = reinterpret_cast<Console*>(user_data);
            console->lines.push_back(message);
            while (console->lines.size() > 64)
            {
                console->lines.pop_front();
            }
        }

        void clear()
        {
            lines.clear();
        }

        bool empty()
        {
            return lines.empty();
        }

    private:
        std::deque<loguru::Message> lines;
        friend void DrawMenu();
    };

    extern Console console;
}
