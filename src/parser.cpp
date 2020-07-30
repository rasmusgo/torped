#include <sstream>

#include "logging.hpp"
#include "parser.hpp"
#include "physfsstruct.hpp"

using namespace std;

int Parser::Load(const char *filename)
{
    // TODO: Make a separate service-thread that loads files and make a list
    //       of what's supposed to happen when the file is loaded. This should
    //       be used for all major file access including loading of textures.

    VLOG_S(3) << "Beginning of Parser::Load()";

    lines.clear();
    namesIndex.clear();
    typeCount.clear();

    // open file
    //ifstream file(filename);
    {
        AutoPHYSFS_file f( PHYSFS_openRead(filename) );
        char c;

        /*
        if (!file.good())
        {
            App::console << "Parser::Load(): Unable to open file: \"" << filename << "\"" << endl;
            return 0;
        }
        */
        if ( f.f == NULL )
            return 0;
        /*
        // store the file line by line
        while (file.good())
        {
            lines.resize(lines.size()+1);
            getline(file, lines.back());
        }
        */

        //TODO: this probably needs optimizations
        lines.resize(lines.size()+1);
        while(PHYSFS_read(f.f, &c, 1, 1))
        {
            if (c == '\n')
            {
                lines.resize(lines.size()+1);
                continue;
            }
            lines.back() += c;
        }
    }
    // close file
    //file.close();

    TypeName typeName;
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        stringstream translator;
        translator << lines[i];
        if(!(translator >> typeName.type))
            continue;
        if(!(translator >> typeName.name))
        {
            LOG_S(WARNING) << "Parser::Load(): " << filename << ": " << i << ": invalid line";
            continue;
        }
        namesIndex[typeName] = typeCount[typeName.type];
        typeCount[typeName.type]++;
    }

    VLOG_S(3) << "End of Parser::Load()";

    return 1; // success
}
