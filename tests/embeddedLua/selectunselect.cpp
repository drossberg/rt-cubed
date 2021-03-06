/*                        T E S T 1 . C P P
 * BRL-CAD
 *
 * Copyright (c) 2018 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/** @file test1.cpp
 *
 *  BRL-CAD embedded lua script:
 *      Select/UnSelectAll test application
 */

#include <iostream>
#include <sstream>

#include <brlcad/MemoryDatabase.h>
#include <embeddedlua.h>


using namespace BRLCAD;


int main
(
    int   argc,
    char* argv[]
) {
    int ret = 0;

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <BRL-CAD Database> <objectToSelect>" << std::endl;
        ret = 1;
    }
    else {
        try {
            std::stringstream TheLuaScript;
            MemoryDatabase database;

            if (database.Load(argv[1])) {
                EmbeddedLuaHandle* handle = CreateEmbeddedLuaHandleInstance(database, 0, 0);

                if (handle != 0) {
                    TheLuaScript << "BRLCAD.database:Select(\"" << argv[2] << "\")\n";
                    handle->Execute(TheLuaScript.str().c_str(), "select test");

                    if (!database.SelectionIsEmpty())
                        std::cout << "Successfully selected \"" << argv[2] << "\""<< std::endl;
                    else
                        std::cerr << "Could not select object \"" << argv[2] << "\"" << std::endl;

                    TheLuaScript.clear();
                    TheLuaScript.str("");

                    TheLuaScript << "BRLCAD.database:UnSelectAll()\n";
                    handle->Execute(TheLuaScript.str().c_str(), "unselect test");

                    if (database.SelectionIsEmpty())
                        std::cout << "Successfully unselected all objects" << std::endl;
                    else
                        std::cerr << "Could not unselect all objects" << std::endl;

                    DestroyEmbeddedLuaHandleInstance(handle);
                }
                else {
                    std::cerr << "Could not create BRL-CAD embedded Lua handle" << std::endl;
                    ret = 2;
                }
            }
            else {
                std::cerr << "Could not load file: " << argv[1] << std::endl;
                ret = 3;
            }
        }
        catch(BRLCAD::bad_alloc& e) {
            std::cerr << "Out of memory in: " << e.what() << std::endl;
            ret = 4;
        }
    }

    return ret;
}
