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
        // This is like a deep clone of a loguru::Message.
        struct Message
        {
            loguru::Verbosity verbosity;   // Already part of preamble
            const std::string filename;    // Already part of preamble
            unsigned          line;        // Already part of preamble
            const std::string preamble;    // Date, time, uptime, thread, file:line, verbosity.
            const std::string indentation; // Just a bunch of spacing.
            const std::string prefix;      // Assertion failure info goes here (or "").
            const std::string message;     // User message goes here.
            Message(const loguru::Message& m)
                : verbosity(m.verbosity)
                , filename(m.filename)
                , line(m.line)
                , preamble(m.preamble)
                , indentation(m.indentation)
                , prefix(m.prefix)
                , message(m.message)
            {}
        };
        std::deque<Message> lines;
        friend void DrawMenu();
    };

    extern Console console;
}
