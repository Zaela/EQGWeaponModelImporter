
#ifndef PTS_H
#define PTS_H

#include <lua.hpp>
#include "types.h"
#include "util.h"

namespace PTS
{
	void LoadFunctions(lua_State* L);

	struct Header
	{
		char magic[4]; //EQPT
		uint32 data_count;
		uint32 version;
		static const uint32 SIZE = sizeof(uint32) * 3;
	};

	struct Data
	{
		char particle_name[64];
		char attach_name[64];
		float translation[3];
		float rotation[3];
		float scale[3];
		static const uint32 SIZE = 128 + sizeof(float) * 9;
	};
}

#endif//PTS_H
