
#include "prt.h"

namespace PRT
{
	void PackV4(lua_State* L, DataV4* data)
	{
		lua_pushinteger(L, data->particle_id);
		lua_setfield(L, -2, "particle_id");
		lua_pushstring(L, data->particle_name);
		lua_setfield(L, -2, "particle_name");

		lua_pushinteger(L, data->duration);
		lua_setfield(L, -2, "duration");
		//packing all the unknowns together for now
		lua_createtable(L, 8, 0);

		int i;
		for (i = 1; i <= 5; ++i)
		{
			lua_pushinteger(L, i);
			lua_pushinteger(L, data->unknownA[i - 1]);
			lua_settable(L, -3);
		}
		lua_pushinteger(L, i++);
		lua_pushinteger(L, data->unknownB);
		lua_settable(L, -3);
		lua_pushinteger(L, i++);
		lua_pushinteger(L, data->unknownFFFFFFFF);
		lua_settable(L, -3);
		lua_pushinteger(L, i);
		lua_pushinteger(L, data->unknownC);
		lua_settable(L, -3);
		
		lua_setfield(L, -2, "unknown");
	}

	int Read(lua_State* L)
	{
		byte* ptr = Util::CheckHeader(L, "PTCL", ".prt");

		Header* header = (Header*)ptr;
		uint32 pos = Header::SIZE;

		lua_createtable(L, header->particle_count, 0); //to return

		if (header->version == 4)
		{
			for (uint32 i = 1; i <= header->particle_count; ++i)
			{
				lua_pushinteger(L, i);
				lua_createtable(L, 11, 0);

				DataV4* data = (DataV4*)&ptr[pos];
				pos += DataV4::SIZE;

				PackV4(L, data);

				lua_settable(L, -3);
			}
		}
		else if (header->version == 5)
		{
			for (uint32 i = 1; i <= header->particle_count; ++i)
			{
				lua_pushinteger(L, i);
				lua_createtable(L, 12, 0);

				DataV5* data = (DataV5*)&ptr[pos];
				pos += DataV5::SIZE;

				PackV4(L, data);

				lua_pushinteger(L, data->particle_id2);
				lua_setfield(L, -2, "particle_id2");

				lua_settable(L, -3);
			}
		}

		return 1;
	}

	int Write(lua_State* L)
	{
		//need to decide whether to write as v4 or v5
		return 1;
	}

	static const luaL_Reg funcs[] = {
		{"Read", Read},
		{"Write", Write},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		Util::LoadFunctions(L, funcs, "prt");
	}
}
