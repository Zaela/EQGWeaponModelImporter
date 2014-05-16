
#include "util.h"

namespace Util
{
	byte* CheckHeader(lua_State* L, const char* magic, const char* err_name)
	{
		if (!lua_istable(L, 1))
		{
			char msg[256];
			snprintf(msg, 256, "expected unprocessed %s data table", err_name);
			luaL_argerror(L, 1, msg);
		}

		lua_getfield(L, 1, "ptr");
		if (!lua_isuserdata(L, -1))
		{
			char msg[256];
			snprintf(msg, 256, "no 'ptr' in unprocessed %s data table", err_name);
			luaL_argerror(L, 1, msg);
		}
		byte* ptr = (byte*)lua_touserdata(L, -1);
		lua_pop(L, 1);

		const char* m = (const char*)ptr;

		if (m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
		{
			char msg[256];
			snprintf(msg, 256, "data is not a valid %s file", err_name);
			luaL_argerror(L, 1, msg);
		}

		return ptr;
	}

	void PrepareWrite(lua_State* L, const char* err_name)
	{
		if (!lua_istable(L, 1))
		{
			char msg[256];
			snprintf(msg, 256, "expected processed %s data table", err_name);
			luaL_argerror(L, 1, msg);
		}
		const char* name = luaL_checkstring(L, 2);
		uint32 crc = luaL_checkinteger(L, 3);

		//ptr, inflated_len, decompressed, name, name crc
		lua_createtable(L, 0, 5); //to return

		lua_pushboolean(L, 1);
		lua_setfield(L, -2, "decompressed");
		lua_pushstring(L, name);
		lua_setfield(L, -2, "name");
		lua_pushinteger(L, crc);
		lua_setfield(L, -2, "crc");
	}

	int FinishWrite(lua_State* L, Buffer& buf)
	{
		byte* ptr = buf.Take();
		lua_pushlightuserdata(L, ptr);
		lua_setfield(L, -2, "ptr");
		lua_pushinteger(L, buf.GetLen());
		lua_setfield(L, -2, "inflated_len");
		return 1;
	}

	double FloatToDouble(float val)
	{
		//this avoids extended precision mucking up nice round floats for display purposes (e.g. 0.7 -> 0.6999999997862...)
		//even though it really shouldn't...
		char str[256];
		snprintf(str, 256, "%g", val);
		return strtod(str, nullptr);
	}

	uint32 GetInt(lua_State* L, int index, const char* name)
	{
		uint32 val = 0;
		lua_getfield(L, index, name);
		val |= lua_tointeger(L, -1);
		lua_pop(L, 1);
		return val;
	}

	float GetFloat(lua_State* L, int index, const char* name)
	{
		lua_getfield(L, index, name);
		float val = static_cast<float>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return val;
	}

	const char* GetString(lua_State* L, int index, const char* name, uint32& len)
	{
		lua_getfield(L, index, name);
		const char* str = lua_tostring(L, -1);
		len = lua_objlen(L, -1) + 1;
		lua_pop(L, 1);
		return str;
	}

	const char* GetString(lua_State* L, int index, const char* name)
	{
		lua_getfield(L, index, name);
		const char* str = lua_tostring(L, -1);
		lua_pop(L, 1);
		return str;
	}


	Buffer::Buffer() : mLen(0), mCap(8192)
	{
		mData = new byte[8192];
	}

	void Buffer::Add(const void* in_data, uint32 len)
	{
		const byte* data = (byte*)in_data;
		uint32 newlen = mLen + len;
		if (newlen >= mCap)
		{
			while (newlen >= mCap)
				mCap <<= 1;
			byte* add = new byte[mCap];
			memcpy(add, mData, mLen);
			delete[] mData;
			mData = add;
		}
		memcpy(&mData[mLen], data, len);
		mLen = newlen;
	}

	byte* Buffer::Take()
	{
		byte* ret = mData;
		mData = nullptr;
		return ret;
	}
}
