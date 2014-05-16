
#include <lua.hpp>
#include <cstdio>
#include <cstring>
#include "types.h"
#include "util.h"

namespace MOD
{
	void LoadFunctions(lua_State* L);

	struct Header
	{
		char magic[4]; //EQGM
		uint32 version;
		uint32 strings_len;
		uint32 material_count;
		uint32 vertex_count;
		uint32 triangle_count;
		uint32 bone_count;
		static const uint32 SIZE = sizeof(uint32) * 7; //in case of 64bit alignment
	};

	struct Material
	{
		uint32 index;
		uint32 name_index;
		uint32 opaque_index;
		uint32 property_count;
		static const uint32 SIZE = sizeof(uint32) * 4;
	};

	struct Property
	{
		uint32 name_index;
		uint32 type;
		uint32 value; //may be int of float depending on the name, will send both representations to lua
		static const uint32 SIZE = sizeof(uint32) * 3;
	};

	struct Vertex
	{
		float x, y, z;
		float i, j, k;
		float u, v;
		static const uint32 SIZE = sizeof(float) * 8;
	};

	struct VertexV3
	{
		float x, y, z;
		float i, j, k;
		uint32 unknown_int;
		float unknown_float[2];
		float u, v;
		static const uint32 SIZE = sizeof(float) * 11;
	};

	struct Triangle
	{
		uint32 index[3];
		uint32 group;
		uint32 flag;
		static const uint32 SIZE = sizeof(uint32) * 5;
	};

	struct Bone
	{
		uint32 name_index;
		float unknown[13];
		static const uint32 SIZE = sizeof(float) * 14;
	};

	struct BoneAssignment //not 100% that's what this is, but it would make sense
	{
		uint32 unknown[9];
		static const uint32 SIZE = sizeof(uint32) * 9;
	};
}
