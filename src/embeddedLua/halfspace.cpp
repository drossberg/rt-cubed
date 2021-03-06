/*                         H A L F S P A C E . C P P
 * BRL-CAD
 *
 * Copyright (c) 2019 United States Government as represented by
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
/** @file halfspace.cpp
 *
 *  BRL-CAD embedded lua script:
 *      BRLCAD::Halfspace functions
 */

#include <cassert>

#include "objectbase.h"
#include "vector3d.h"
#include "halfspace.h"


struct ScriptHalfspace {
    BRLCAD::Halfspace* object;
    bool               own;
};


static BRLCAD::Halfspace& GetHalfspace
(
    lua_State* luaState,
    int        narg
) {
    BRLCAD::Halfspace* object = TestHalfspace(luaState, narg);
    assert(object != 0);

    return *object;
}


static int CreateHalfspace
(
    lua_State* luaState
) {
    BRLCAD::Halfspace* ret = 0;

    if (lua_gettop(luaState) > 0) {
        BRLCAD::Halfspace* original = TestHalfspace(luaState, 1);

        if (original != 0)
            ret = new BRLCAD::Halfspace(*original);
        else {
            BRLCAD::Vector3D normal             = GetVector3D(luaState, 1);
            double           distanceFromOrigin = luaL_checknumber(luaState, 2);

            ret = new BRLCAD::Halfspace(normal, distanceFromOrigin);
        }
    }

    if (ret == 0)
        ret = new BRLCAD::Halfspace();

    return PushHalfspace(luaState, ret, true);
}


static int Destruct
(
    lua_State* luaState
) {
    ScriptHalfspace* scriptObject = static_cast<ScriptHalfspace*>(luaL_testudata(luaState, 1, "BRLCAD.Halfspace"));

    if ((scriptObject != 0) && (scriptObject->object != 0) && scriptObject->own)
        scriptObject->object->Destroy();

    return 0;
}


static int Normal
(
    lua_State* luaState
) {
    BRLCAD::Halfspace& object = GetHalfspace(luaState, 1);

    PushVector3D(luaState, object.Normal());

    return 1;
}


static int SetNormal
(
    lua_State* luaState
) {
    BRLCAD::Halfspace& object = GetHalfspace(luaState, 1);
    BRLCAD::Vector3D   normal = GetVector3D(luaState, 2);

    object.SetNormal(normal);

    return 0;
}


static int DistanceFromOrigin
(
    lua_State* luaState
) {
    BRLCAD::Halfspace& object = GetHalfspace(luaState, 1);

    lua_pushnumber(luaState, object.DistanceFromOrigin());

    return 1;
}


static int SetDistanceFromOrigin
(
    lua_State* luaState
) {
    BRLCAD::Halfspace& object = GetHalfspace(luaState, 1);
    double             value = luaL_checknumber(luaState, 2);

    object.SetDistanceFromOrigin(value);

    return 0;
}


static int Set
(
    lua_State* luaState
) {
    BRLCAD::Halfspace& object             = GetHalfspace(luaState, 1);
    BRLCAD::Vector3D   normal             = GetVector3D(luaState, 2);
    double             distanceFromOrigin = luaL_checknumber(luaState, 3);

    object.Set(normal, distanceFromOrigin);

    return 0;
}


static int ClassName
(
    lua_State* luaState
) {
    lua_pushstring(luaState, BRLCAD::Halfspace::ClassName());

    return 1;
}


static BRLCAD::Object& GetObject
(
    lua_State* luaState,
    int        narg
) {
    return GetHalfspace(luaState, narg);
}


static void GetHalfspaceMetatable
(
    lua_State* luaState
) {
    lua_getfield(luaState, LUA_REGISTRYINDEX, "BRLCAD.Halfspace");

    if (!lua_istable(luaState, -1)) {
        lua_pop(luaState, 1);
        luaL_newmetatable(luaState, "BRLCAD.Halfspace");

        lua_pushvalue(luaState, -1);
        lua_setfield(luaState, -2, "__index");

        lua_pushstring(luaState, "__gc");
        lua_pushcfunction(luaState, &Destruct);
        lua_settable(luaState, -3);

        PushObjectMetatable<GetObject>(luaState);

        lua_pushstring(luaState, "Normal");
        lua_pushcfunction(luaState, &Normal);
        lua_settable(luaState, -3);

        lua_pushstring(luaState, "SetNormal");
        lua_pushcfunction(luaState, &SetNormal);
        lua_settable(luaState, -3);

        lua_pushstring(luaState, "DistanceFromOrigin");
        lua_pushcfunction(luaState, &DistanceFromOrigin);
        lua_settable(luaState, -3);

        lua_pushstring(luaState, "SetDistanceFromOrigin");
        lua_pushcfunction(luaState, &SetDistanceFromOrigin);
        lua_settable(luaState, -3);

        lua_pushstring(luaState, "Set");
        lua_pushcfunction(luaState, &Set);
        lua_settable(luaState, -3);

        lua_pushstring(luaState, "Clone");
        lua_pushcfunction(luaState, &CreateHalfspace);
        lua_settable(luaState, -3);

        lua_pushstring(luaState, "ClassName");
        lua_pushcfunction(luaState, &ClassName);
        lua_settable(luaState, -3);
    }
}


void InitHalfspace
(
    lua_State* luaState
) {
    lua_getglobal(luaState, "BRLCAD");

    if (!lua_istable(luaState, -1)) {
        lua_pop(luaState, 1);
        lua_newtable(luaState);
        lua_setglobal(luaState, "BRLCAD");
        lua_getglobal(luaState, "BRLCAD");
    }

    lua_pushcfunction(luaState, &CreateHalfspace);
    lua_setfield(luaState, -2, "Halfspace");

    lua_pop(luaState, 1);
}


int PushHalfspace
(
    lua_State*         luaState,
    BRLCAD::Halfspace* object,
    bool               takeOwnership
) {
    int ret = 0;

    if (object != 0) {
        ScriptHalfspace* scriptObject = static_cast<ScriptHalfspace*>(lua_newuserdata(luaState, sizeof(ScriptHalfspace)));

        scriptObject->object = object;
        scriptObject->own    = takeOwnership;

        GetHalfspaceMetatable(luaState);
        lua_setmetatable(luaState, -2);

        ret = 1;
    }

    return ret;
}


BRLCAD::Halfspace* TestHalfspace
(
    lua_State* luaState,
    int        narg
) {
    BRLCAD::Halfspace* ret          = 0;
    ScriptHalfspace*   scriptObject = static_cast<ScriptHalfspace*>(luaL_testudata(luaState, narg, "BRLCAD.Halfspace"));

    if (scriptObject != 0)
        ret = scriptObject->object;

    return ret;
}
