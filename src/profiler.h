#ifndef PROFILER_H
#define PROFILER_H

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/nommgr.h"
#endif

#include <chrono>
#include <vector>
#include <string>

#ifdef MEMORY_MANAGER
#include "Fluid_Studios_Memory_Manager/mmgr.h"
#endif


class Profiler
{
public:
    Profiler()
    {
        last_time = std::chrono::high_resolution_clock::now();
    }

    Profiler(const Profiler &p)
    {
        times     = p.times;
        names     = p.names;
        last_time = p.last_time;
    }

    void operator = (const Profiler &p)
    {
        times     = p.times;
        names     = p.names;
        last_time = p.last_time;
    }

    void swap(Profiler &p)
    {
        times.swap(p.times);
        names.swap(p.names);
    }

    void Reset()
    {
        names.clear();
        times.clear();
        last_time = std::chrono::high_resolution_clock::now();
    }

    void SkipTime()
    {
        last_time = std::chrono::high_resolution_clock::now();
    }

    void RememberTime(std::string name)
    {
        names.push_back(name);
        auto t = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t - last_time).count());
        last_time = t;
    }

    void AddTime(std::string name)
    {
        // Find the previous record (traverse backwards)
        typeof(names.end()) it = names.end();
        typeof(times.end()) it2 = times.end();
        while (it != names.begin())
        {
            --it;
            --it2;
            if (*it == name)
            {
                auto t = std::chrono::high_resolution_clock::now();
                *it2 += std::chrono::duration_cast<std::chrono::nanoseconds>(t - last_time).count();
                last_time = t;
                return;
            }
        }

        // Create a new record if no previous was found
        RememberTime(name);
    }

    Profiler operator + (const Profiler &p) const
    {
        Profiler ret;
        unsigned int i=0, j=0;

        while ( i < names.size() )
        {
            // Is it the same name?
            if (names[i] == p.names[j])
            {
                // Insert the sum of time
                ret.names.push_back(names[i]);
                ret.times.push_back(times[i] + p.times[j]);
                ++i;
                ++j;
                continue;
            }

            // If not, try to find it
            for (unsigned int k = j; k < p.names.size(); ++k)
            {
                if (names[i] == p.names[k])
                {
                    // Insert all names and times found in-between
                    for (; j < k; ++j)
                    {
                        ret.names.push_back(p.names[j]);
                        ret.times.push_back(p.times[j]);
                    }
                    // Insert the matching name and sum of time
                    ret.names.push_back(names[i]);
                    ret.times.push_back(times[i] + p.times[j]);
                    ++i;
                    ++j;
                    goto end_of_while;
                }
            }

            // names[i] could not be found in p.names, insert it as it is
            ret.names.push_back(names[i]);
            ret.times.push_back(times[i]);
            ++i;

        end_of_while:;
        }

        // Insert all remaining names and times from p
        while ( j < p.names.size() )
        {
            ret.names.push_back(p.names[j]);
            ret.times.push_back(p.times[j]);
            ++j;
        }

        if (last_time > p.last_time)
            ret.last_time = last_time;
        else
            ret.last_time = p.last_time;

        return ret;
    }

    void operator += (const Profiler &p)
    {
        unsigned int i=0, j=0;

        while ( i < names.size() && j < p.names.size() )
        {
            // Is it the same name?
            if (names[i] == p.names[j])
            {
                // Insert the sum of time
                times[i] += p.times[j];
                ++i;
                ++j;
                continue;
            }

            // If not, try to find it
            for (unsigned int k = j; k < p.names.size(); ++k)
            {
                if (names[i] == p.names[k])
                {
                    // Insert all names and times found in-between
                    for (; j < k; ++j)
                    {
                        names.insert(names.begin() + i, p.names[j]);
                        times.insert(times.begin() + i, p.times[j]);
                        ++i;
                    }
                    // Insert the matching name and sum of time
                    times[i] += p.times[j];
                    ++i;
                    ++j;
                    goto end_of_while;
                }
            }

            ++i;

        end_of_while:;
        }

        // Insert all remaining names and times from p
        while ( j < p.names.size() )
        {
            names.push_back(p.names[j]);
            times.push_back(p.times[j]);
            ++j;
        }
    }

    std::vector<size_t> times;
    std::vector<std::string> names;
    decltype(std::chrono::high_resolution_clock::now()) last_time;
};

#endif // PROFILER_H
