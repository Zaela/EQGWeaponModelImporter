
#ifndef UTIL_H
#define UTIL_H

#include "types.h"
#include <cstdio>
#include <cstring>
#include <lua.hpp>

namespace Util
{
	class Buffer;

	byte* CheckHeader(lua_State* L, const char* magic, const char* err_name);
	void PrepareWrite(lua_State* L, const char* err_name);
	int FinishWrite(lua_State* L, Buffer& buf);

	double FloatToDouble(float val);
	uint32 GetInt(lua_State* L, int index, const char* name);
	uint32 GetInt(lua_State* L, int index, int pos);
	float GetFloat(lua_State* L, int index, const char* name);
	const char* GetString(lua_State* L, int index, const char* name, uint32& len);
	const char* GetString(lua_State* L, int index, const char* name);

	class Buffer
	{
	public:
		Buffer();
		void Add(const void* in_data, uint32 len);
		byte* Take();
		uint32 GetLen() { return mLen; }
	private:
		uint32 mLen;
		uint32 mCap;
		byte* mData;
	};
}

#endif//UTIL_H