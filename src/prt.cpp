
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
		lua_pushinteger(L, 6);
		lua_pushinteger(L, data->unknownB);
		lua_settable(L, -3);
		lua_pushinteger(L, 7);
		lua_pushinteger(L, data->unknownFFFFFFFF);
		lua_settable(L, -3);
		lua_pushinteger(L, 8);
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
		//takes a prt data table, returns a .eqg directory entry table
		Util::PrepareWrite(L, ".prt");

		Header header;
		header.magic[0] = 'P';
		header.magic[1] = 'T';
		header.magic[2] = 'C';
		header.magic[3] = 'L';
		header.version = 4;
		header.particle_count = lua_objlen(L, 1);

		Util::Buffer buf;
		buf.Add(&header, Header::SIZE);

		for (uint32 i = 1; i <= header.particle_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, 1);

			DataV4 d;
			d.particle_id = Util::GetInt(L, 1, "particle_id");
			snprintf(d.particle_name, 64, "%s", Util::GetString(L, 1, "particle_name"));
			d.duration = Util::GetInt(L, 1, "duration");

			lua_getfield(L, 1, "unknown");
			for (int j = 1; j <= 5; ++j)
			{
				d.unknownA[j - 1] = Util::GetInt(L, -2, j);
			}
			d.unknownB = Util::GetInt(L, -2, 6);
			lua_pushinteger(L, 7);
			lua_gettable(L, -2);
			d.unknownFFFFFFFF = lua_tointeger(L, -1);
			lua_pop(L, 1);
			d.unknownC = Util::GetInt(L, -2, 8);
			lua_pop(L, 1);

			buf.Add(&d, DataV4::SIZE);

			lua_pop(L, 1);
		}

		return Util::FinishWrite(L, buf);
	}

	static const luaL_Reg funcs[] = {
		{"Read", Read},
		{"Write", Write},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		luaL_register(L, "prt", funcs);
	}
}
