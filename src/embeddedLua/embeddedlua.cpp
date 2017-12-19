/*                  E M B E D D E D L U A . C P P
 * BRL-CAD
 *
 * Copyright (c) 2017 United States Government as represented by
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
/** @file embeddedlua.cpp
 *
 *  BRL-CAD embedded lua script:
 *      API declaration
 */

#include <cstring>

#include "lua.hpp"

#include "writestring.h"
#include "initbrlcad.h"
#include "embeddedlua.h"


using namespace BRLCAD;


BRLCAD_EMBEDDEDLUA_EXPORT bool BRLCAD::RunEmbeddedLua
(
    Database&   database,
    const char* script,
    void        (*stdOut)(const char* text),
    void        (*stdErr)(const char* text)
) {
    bool ret = true;

    if (script != 0) {
        brlcad_stdoutstream = stdOut;
        brlcad_sterrstream  = stdErr;

        lua_State* luaState = luaL_newstate();

        if (luaState != 0) {
            luaL_openlibs(luaState);
            InitBrlcad(luaState, database);

            int error = luaL_loadbuffer(luaState, script, strlen(script), "BRL-CAD");

            if (error == 0)
                lua_call(luaState, 0, 0);
            else {
                ret = false;

                switch (error) {
                case LUA_ERRSYNTAX:
                    if (stdErr != 0)
                        stdErr("Lua syntax error: ");

                    break;

                case LUA_ERRMEM:
                    if (stdErr != 0)
                        stdErr("Lua memory allocation error: ");

                    break;

                default:
                    if (stdErr != 0)
                        stdErr("Unknown lua_load return code: ");

                    break;
                }

                const char* logMessage = lua_tostring(luaState, -1);

                stdErr(logMessage);
                stdErr("\n");
                lua_pop(luaState, 1);
            }

            lua_close(luaState);
        }
        else {
            if (stdErr != 0)
                stdErr("Could not open Lua\n");

            ret = false;
        }

        brlcad_stdoutstream = 0;
        brlcad_sterrstream  = 0;
    }

    return ret;
}


class EmbeddedLuaHandleImplementation : public EmbeddedLuaHandle {
public:
    EmbeddedLuaHandleImplementation(Database& database,
                                    void      (*stdOut)(const char* text),
                                    void      (*stdErr)(const char* text)) : EmbeddedLuaHandle(), m_stdOut(stdOut), m_stdErr(stdErr) {
        m_luaState = luaL_newstate();

        if (m_luaState != 0) {
            luaL_openlibs(m_luaState);
            InitBrlcad(m_luaState, database);
        }
        else if (stdErr != 0)
            stdErr("Could not open Lua\n");
    }

    virtual ~EmbeddedLuaHandleImplementation(void) {
        if (m_luaState != 0)
            lua_close(m_luaState);
    }

    virtual bool Execute(const char* script) {
        bool ret = true;

        if (script != 0) {
            brlcad_stdoutstream = m_stdOut;
            brlcad_sterrstream  = m_stdErr;

            if (m_luaState != 0) {
                int error = luaL_loadbuffer(m_luaState, script, strlen(script), "BRL-CAD");

                if (error == 0)
                    lua_call(m_luaState, 0, 0);
                else {
                    ret = false;

                    switch (error) {
                    case LUA_ERRSYNTAX:
                        if (m_stdErr != 0)
                            m_stdErr("Lua syntax error: ");

                        break;

                    case LUA_ERRMEM:
                        if (m_stdErr != 0)
                            m_stdErr("Lua memory allocation error: ");

                        break;

                    default:
                        if (m_stdErr != 0)
                            m_stdErr("Unknown lua_load return code: ");

                        break;
                    }

                    const char* logMessage = lua_tostring(m_luaState, -1);

                    m_stdErr(logMessage);
                    m_stdErr("\n");
                    lua_pop(m_luaState, 1);
                }
            }
            else
                ret = false;

            brlcad_stdoutstream = 0;
            brlcad_sterrstream  = 0;
        }

        return ret;
    }

private:
    lua_State* m_luaState;
    void       (*m_stdOut)(const char* text);
    void       (*m_stdErr)(const char* text);
};


BRLCAD_EMBEDDEDLUA_EXPORT EmbeddedLuaHandle* BRLCAD::CreateEmbeddedLuaHandleInstance
(
    Database& database,
    void      (*stdOut)(const char* text),
    void      (*stdErr)(const char* text)
) {
    return new EmbeddedLuaHandleImplementation(database, stdOut, stdErr);
}


BRLCAD_EMBEDDEDLUA_EXPORT void BRLCAD::DestroyEmbeddedLuaHandleInstance
(
    EmbeddedLuaHandle* handle
) {
    delete handle;
}