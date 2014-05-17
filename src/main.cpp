
#include <lua.hpp>
#include <cstdio>

#include <iup.h>
#include <iuplua.h>
#include <iupcontrols.h>
#include <iupluacontrols.h>

#include "mod.h"
#include "prt.h"
#include "pts.h"

#ifdef _WIN32
#include <windows.h>

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#else
int main()
#endif
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	IupOpen(nullptr, nullptr);
	IupControlsOpen();
	iuplua_open(L);
	iupcontrolslua_open(L);

	MOD::LoadFunctions(L);
	PRT::LoadFunctions(L);
	PTS::LoadFunctions(L);

	if (luaL_loadfile(L, "gui/main.lua") != 0)
	{
#ifdef _WIN32
		char msg[1024];
		snprintf(msg, 1024, "Could not load GUI script:\n%s", lua_tostring(L, -1));
		MessageBox(NULL, msg, NULL, MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
		printf("Error: Could not load GUI script: %s\n", lua_tostring(L, -1));
#endif
	}
	else if (lua_pcall(L, 0, 0, 0) != 0)
	{
#ifdef _WIN32
		char msg[1024];
		snprintf(msg, 1024, "Runtime error:\n%s", lua_tostring(L, -1));
		MessageBox(NULL, msg, NULL, MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
		printf("Runtime error: %s\n", lua_tostring(L, -1));
#endif
	}

	lua_close(L);
	return 0;
}
