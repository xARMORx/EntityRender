#pragma once
/*
I am not a complete author of this code. I just rewrote it and added Lua Api
Code autor: ARMOR https://www.blast.hk/members/287503/
Original code: https://gist.github.com/imring/a6f5f3ec629bf23aba3dbfeb9f2dd84c
*/

#define LUAOPEN_MODULE_EXPAND(name) luaopen_##name
#define LUAOPEN_MODULE(name) LUAOPEN_MODULE_EXPAND name
#define LUA_MODULE_ENTRYPOINT extern "C" __declspec(dllexport) int LUAOPEN_MODULE((MODULE_NAME))
#define SOL_MODULE_ENTRYPOINT(func) LUA_MODULE_ENTRYPOINT(lua_State* L) { return (sol::c_call<decltype(&func), &func>)(L); }

#pragma warning(disable : 26495)
#pragma warning(disable : 26819)
#pragma warning(disable : 26439)
#pragma warning(disable : 26800)
#include "sol2/sol.hpp"
#pragma warning(default : 26800)
#pragma warning(default : 26439)
#pragma warning(default : 26819)
#pragma warning(default : 26495)

#include "CEntityRender.h"