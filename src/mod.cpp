
#include "mod.h"

namespace MOD
{
	int Read(lua_State* L)
	{
		byte* ptr = Util::CheckHeader(L, "EQGM", ".mod");

		Header* header = (Header*)ptr;
		uint32 pos = Header::SIZE;

		lua_createtable(L, 0, 7); //to return

		const char* string_block = (const char*)&ptr[pos];
		pos += header->strings_len;

		//materials and their properties
		lua_createtable(L, header->material_count, 0);

		for (uint32 i = 1; i <= header->material_count; ++i)
		{
			Material* mat = (Material*)&ptr[pos];
			pos += Material::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, mat->property_count, 2); //one table per material

			lua_pushstring(L, &string_block[mat->name_index]);
			lua_setfield(L, -2, "name");
			lua_pushstring(L, &string_block[mat->opaque_index]);
			lua_setfield(L, -2, "opaque");

			for (uint32 j = 1; j <= mat->property_count; ++j)
			{
				Property* prop = (Property*)&ptr[pos];
				pos += Property::SIZE;

				lua_pushinteger(L, j);
				lua_createtable(L, 0, 3);

				lua_pushstring(L, &string_block[prop->name_index]);
				lua_setfield(L, -2, "name");
				lua_pushinteger(L, prop->type);
				lua_setfield(L, -2, "type");
				if (prop->type == 0)
					lua_pushnumber(L, Util::FloatToDouble(*(float*)&prop->value));
				else
					lua_pushstring(L, &string_block[prop->value]);
				lua_setfield(L, -2, "value");

				lua_settable(L, -3);
			}

			lua_settable(L, -3);
		}

		lua_setfield(L, -2, "materials");

		//vertices
		lua_createtable(L, header->vertex_count, 0);

		if (header->version < 3)
		{
			for (uint32 i = 1; i <= header->vertex_count; ++i)
			{
				Vertex* vert = (Vertex*)&ptr[pos];
				pos += Vertex::SIZE;

				lua_pushinteger(L, i);
				lua_createtable(L, 0, 8);

				lua_pushnumber(L, Util::FloatToDouble(vert->x));
				lua_setfield(L, -2, "x");
				lua_pushnumber(L, Util::FloatToDouble(vert->y));
				lua_setfield(L, -2, "y");
				lua_pushnumber(L, Util::FloatToDouble(vert->z));
				lua_setfield(L, -2, "z");
				lua_pushnumber(L, Util::FloatToDouble(vert->i));
				lua_setfield(L, -2, "i");
				lua_pushnumber(L, Util::FloatToDouble(vert->j));
				lua_setfield(L, -2, "j");
				lua_pushnumber(L, Util::FloatToDouble(vert->k));
				lua_setfield(L, -2, "k");
				lua_pushnumber(L, Util::FloatToDouble(vert->u));
				lua_setfield(L, -2, "u");
				lua_pushnumber(L, Util::FloatToDouble(vert->v));
				lua_setfield(L, -2, "v");

				lua_settable(L, -3);
			}
		}
		else
		{
			for (uint32 i = 1; i <= header->vertex_count; ++i)
			{
				VertexV3* vert = (VertexV3*)&ptr[pos];
				pos += VertexV3::SIZE;

				lua_pushinteger(L, i);
				lua_createtable(L, 0, 8);

				lua_pushnumber(L, Util::FloatToDouble(vert->x));
				lua_setfield(L, -2, "x");
				lua_pushnumber(L, Util::FloatToDouble(vert->y));
				lua_setfield(L, -2, "y");
				lua_pushnumber(L, Util::FloatToDouble(vert->z));
				lua_setfield(L, -2, "z");
				lua_pushnumber(L, Util::FloatToDouble(vert->i));
				lua_setfield(L, -2, "i");
				lua_pushnumber(L, Util::FloatToDouble(vert->j));
				lua_setfield(L, -2, "j");
				lua_pushnumber(L, Util::FloatToDouble(vert->k));
				lua_setfield(L, -2, "k");
				lua_pushnumber(L, Util::FloatToDouble(vert->u));
				lua_setfield(L, -2, "u");
				lua_pushnumber(L, Util::FloatToDouble(vert->v));
				lua_setfield(L, -2, "v");

				lua_settable(L, -3);
			}
		}

		lua_setfield(L, -2, "vertices");

		//triangles
		lua_createtable(L, header->triangle_count, 0);

		for (uint32 i = 1; i <= header->triangle_count; ++i)
		{
			Triangle* tri = (Triangle*)&ptr[pos];
			pos += Triangle::SIZE;

			lua_pushinteger(L, i);
			lua_createtable(L, 3, 2);

			lua_pushinteger(L, 1);
			lua_pushinteger(L, tri->index[0]);
			lua_settable(L, -3);
			lua_pushinteger(L, 2);
			lua_pushinteger(L, tri->index[1]);
			lua_settable(L, -3);
			lua_pushinteger(L, 3);
			lua_pushinteger(L, tri->index[2]);
			lua_settable(L, -3);
			lua_pushinteger(L, tri->group);
			lua_setfield(L, -2, "group");
			lua_pushinteger(L, tri->flag);
			lua_setfield(L, -2, "flag");

			lua_settable(L, -3);
		}

		lua_setfield(L, -2, "triangles");

		//bones, if the model has animations...
		lua_createtable(L, header->bone_count, 0);

		for (uint32 i = 1; i <= header->bone_count; ++i)
		{
			Bone* bone = (Bone*)&ptr[pos];
			pos += Bone::SIZE;

			lua_pushinteger(L, i);
			lua_newtable(L);

			lua_pushstring(L, &string_block[bone->name_index]);
			lua_setfield(L, -2, "name");
			//put data here when interpretation is known
			Bone* raw = (Bone*)lua_newuserdata(L, Bone::SIZE);
			*raw = *bone;
			lua_setfield(L, -2, "data");

			lua_settable(L, -3);
		}

		lua_setfield(L, -2, "bones");

		//bone assignments?
		if (header->bone_count > 0)
		{
			lua_createtable(L, header->vertex_count, 0);

			for (uint32 i = 1; i<= header->vertex_count; ++i)
			{
				BoneAssignment* ba = (BoneAssignment*)&ptr[pos];
				pos += BoneAssignment::SIZE;

				lua_pushinteger(L, i);
				//lua_newtable(L);

				//put data here when interpretation is known
				BoneAssignment* raw = (BoneAssignment*)lua_newuserdata(L, BoneAssignment::SIZE);
				*raw = *ba;

				lua_settable(L, -3);
			}
		}
		else
		{
			lua_newtable(L);
		}
		
		lua_setfield(L, -2, "bone_assignments");

		return 1;
	}

	int Write(lua_State* L)
	{
		//takes a mod data table, returns a .eqg directory entry table
		Util::PrepareWrite(L, ".mod");

		//write .mod data
		Header header;
		header.magic[0] = 'E';
		header.magic[1] = 'Q';
		header.magic[2] = 'G';
		header.magic[3] = 'M';
		header.version = 1;

		Util::Buffer name_buf;
		Util::Buffer data_buf;

		lua_getfield(L, 1, "materials");
		header.material_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.material_count; ++i)
		{
			uint32 len;
			const char* name;
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			Material mat;
			mat.index = i - 1;
			
			name = Util::GetString(L, -1, "name", len);
			mat.name_index = name_buf.GetLen();
			name_buf.Add(name, len);
			name = Util::GetString(L, -1, "opaque", len);
			mat.opaque_index = name_buf.GetLen();
			name_buf.Add(name, len);

			mat.property_count = lua_objlen(L, -1);
			data_buf.Add(&mat, Material::SIZE);

			for (uint32 j = 1; j <= mat.property_count; ++j)
			{
				lua_pushinteger(L, j);
				lua_gettable(L, -2);

				Property prop;
				name = Util::GetString(L, -1, "name", len);
				prop.name_index = name_buf.GetLen();
				name_buf.Add(name, len);
				prop.type = Util::GetInt(L, -1, "type");
				if (prop.type == 0)
				{
					float val = Util::GetFloat(L, -1, "value");
					prop.value = *(uint32*)&val;
				}
				else
				{
					name = Util::GetString(L, -1, "value", len);
					prop.value = name_buf.GetLen();
					name_buf.Add(name, len);
				}

				data_buf.Add(&prop, Property::SIZE);
				lua_pop(L, 1);
			}

			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "vertices");
		header.vertex_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.vertex_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			Vertex vert;
			vert.x = Util::GetFloat(L, -1, "x");
			vert.y = Util::GetFloat(L, -1, "y");
			vert.z = Util::GetFloat(L, -1, "z");
			vert.i = Util::GetFloat(L, -1, "i");
			vert.j = Util::GetFloat(L, -1, "j");
			vert.k = Util::GetFloat(L, -1, "k");
			vert.u = Util::GetFloat(L, -1, "u");
			vert.v = Util::GetFloat(L, -1, "v");

			data_buf.Add(&vert, Vertex::SIZE);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "triangles");
		header.triangle_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.triangle_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			Triangle tri;
			for (int j = 0; j < 3; ++j)
			{
				lua_pushinteger(L, j + 1);
				lua_gettable(L, -2);
				tri.index[j] = lua_tointeger(L, -1);
				lua_pop(L, 1);
			}
			tri.group = Util::GetInt(L, -1, "group");
			tri.flag = Util::GetInt(L, -1, "flag");

			data_buf.Add(&tri, Triangle::SIZE);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "bones");
		header.bone_count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= header.bone_count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			lua_getfield(L, -1, "data");
			Bone* raw = (Bone*)lua_touserdata(L, -1);
			lua_pop(L, 1);

			uint32 len;
			const char* name = Util::GetString(L, -1, "name", len);
			raw->name_index = name_buf.GetLen();
			name_buf.Add(name, len);

			data_buf.Add(raw, Bone::SIZE);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "bone_assignments");
		uint32 count = lua_objlen(L, -1);
		for (uint32 i = 1; i <= count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, -2);

			BoneAssignment* raw = (BoneAssignment*)lua_touserdata(L, -1);

			data_buf.Add(raw, BoneAssignment::SIZE);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		header.strings_len = name_buf.GetLen();

		Util::Buffer buf;
		buf.Add(&header, Header::SIZE);

		byte* b = name_buf.Take();
		buf.Add(b, name_buf.GetLen());
		delete[] b;

		b = data_buf.Take();
		buf.Add(b, data_buf.GetLen());
		delete[] b;

		return Util::FinishWrite(L, buf);
	}

	static const luaL_Reg funcs[] = {
		{"Read", Read},
		{"Write", Write},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		Util::LoadFunctions(L, funcs, "mod");
	}
}
