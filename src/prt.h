
#ifndef PRT_H
#define PRT_H

#include <lua.hpp>
#include "types.h"
#include "util.h"

namespace PRT
{
	void LoadFunctions(lua_State* L);

	struct Header
	{
		char magic[4]; //PTCL
		uint32 particle_count;
		uint32 version;
		static const uint32 SIZE = sizeof(uint32) * 3;
	};

	struct DataV4
	{
		uint32 particle_id; //particle id from actorsemittersnew.edd
		char particle_name[64];
		uint32 unknownA[5]; //Pretty sure last 3 have something to do with durations
		uint32 duration; //? usually 5000
		uint32 unknownB; //Always seen as 0
		uint32 unknownFFFFFFFF; //Always seen as FFFFFFFF (Transparency maybe?)
		uint32 unknownC; //Seen 1 or 0, mostly 0
		static const uint32 SIZE = 64 + sizeof(uint32) * 10;
	};

	struct DataV5 : public DataV4
	{
		uint32 particle_id2; //Seen this match particle_id on most newer prt files
		static const uint32 SIZE = DataV4::SIZE + sizeof(uint32);
	};
}

#endif//PRT_H
