#include "../lua/lua.hpp"

extern "C" int luaopen_mylib(lua_State *L)
{
	lua_pushstring(L, "fucklib");
	return 1;
}

