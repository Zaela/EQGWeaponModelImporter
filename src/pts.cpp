
#include "pts.h"

namespace PTS
{
	int Read(lua_State* L)
	{
		byte* ptr = Util::CheckHeader(L, "EQPT", ".pts");

		Header* header = (Header*)ptr;
		uint32 pos = Header::SIZE;

		lua_createtable(L, header->data_count, 0); //to return

		for (uint32 i = 1; i <= header->data_count; ++i)
		{
			Data* d = (Data*)&ptr[pos];
			pos += Data::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, 0, 11);

			lua_pushstring(L, d->particle_name);
			lua_setfield(L, -2, "particle_name");
			lua_pushstring(L, d->attach_name);
			lua_setfield(L, -2, "attach_name");

			//not 100% sure about the x y z order
			//translations
			lua_createtable(L, 0, 3);

			lua_pushnumber(L, Util::FloatToDouble(d->translation[0]));
			lua_setfield(L, -2, "z");
			lua_pushnumber(L, Util::FloatToDouble(d->translation[1]));
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Util::FloatToDouble(d->translation[2]));
			lua_setfield(L, -2, "x");

			lua_setfield(L, -2, "translation");

			//rotations
			lua_createtable(L, 0, 3);

			lua_pushnumber(L, Util::FloatToDouble(d->rotation[0]));
			lua_setfield(L, -2, "z");
			lua_pushnumber(L, Util::FloatToDouble(d->rotation[1]));
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Util::FloatToDouble(d->rotation[2]));
			lua_setfield(L, -2, "x");

			lua_setfield(L, -2, "rotation");

			//scales
			lua_createtable(L, 0, 3);

			lua_pushnumber(L, Util::FloatToDouble(d->scale[0]));
			lua_setfield(L, -2, "z");
			lua_pushnumber(L, Util::FloatToDouble(d->scale[1]));
			lua_setfield(L, -2, "y");
			lua_pushnumber(L, Util::FloatToDouble(d->scale[2]));
			lua_setfield(L, -2, "x");

			lua_setfield(L, -2, "scale");

			lua_settable(L, -3);
		}

		return 1;
	}

	int Write(lua_State* L)
	{
		//takes a pts data table, returns a .eqg directory entry table
		Util::PrepareWrite(L, ".pts");

		//write .pts data
		Header header;
		header.magic[0] = 'E';
		header.magic[1] = 'Q';
		header.magic[2] = 'P';
		header.magic[3] = 'T';
		header.version = 1;
		header.data_count = lua_objlen(L, 1);

		Util::Buffer buf;
		buf.Add(&header, Header::SIZE);

		for (uint32 i = 1; i <= header.data_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, 1);

			Data d;
			snprintf(d.particle_name, 64, "%s", Util::GetString(L, -1, "particle_name"));
			snprintf(d.attach_name, 64, "%s", Util::GetString(L, -1, "attach_name"));

			lua_getfield(L, -1, "translation");
			d.translation[0] = Util::GetFloat(L, -1, "x");
			d.translation[1] = Util::GetFloat(L, -1, "z");
			d.translation[2] = Util::GetFloat(L, -1, "y");
			lua_pop(L, 1);

			lua_getfield(L, -1, "rotation");
			d.rotation[0] = Util::GetFloat(L, -1, "x");
			d.rotation[1] = Util::GetFloat(L, -1, "z");
			d.rotation[2] = Util::GetFloat(L, -1, "y");
			lua_pop(L, 1);

			lua_getfield(L, -1, "scale");
			d.scale[0] = Util::GetFloat(L, -1, "x");
			d.scale[1] = Util::GetFloat(L, -1, "z");
			d.scale[2] = Util::GetFloat(L, -1, "y");
			lua_pop(L, 1);

			buf.Add(&d, Data::SIZE);

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
		Util::LoadFunctions(L, funcs, "pts");
	}
}
