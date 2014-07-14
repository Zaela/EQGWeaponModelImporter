
#include "viewer.h"

using namespace irr;

extern std::thread* gViewerThread;
extern std::atomic<scene::SMeshBuffer*> gViewMesh;
extern std::atomic<Viewer::ImageFile*> gImageFile;
extern std::atomic_flag gRunThread;

namespace Viewer
{
	void ThreadMain(uint32 w, uint32 h)
	{
		IrrlichtDevice* device = createDevice(video::EDT_OPENGL,
			core::dimension2du(w, h), 16, false, false, true);

		if (device == nullptr)
		{
			gRunThread.clear();
			return;
		}

		device->setWindowCaption(L"EQG Weapon Model Viewer");
		scene::ISceneManager* mgr = device->getSceneManager();
		video::IVideoDriver* driver = device->getVideoDriver();

		scene::ICameraSceneNode* cam = mgr->addCameraSceneNodeMaya(nullptr, -100.0f, 10.0f, 2.5f, -1, 5);
		scene::SMeshBuffer* active_buffer = nullptr;
		scene::ISceneNode* node = nullptr;

		for (;;)
		{
			//check if we should close thread
			if (gRunThread.test_and_set() == false)
			{
				device->drop();
				gRunThread.clear();
				gViewMesh.store(nullptr);
				return;
			}

			//check if we have a new model to view
			scene::SMeshBuffer* view_buffer = gViewMesh.load();
			if (view_buffer != active_buffer)
			{
				active_buffer = view_buffer;
				if (node)
					node->remove();

				mgr->setAmbientLight(video::SColorf(1, 1, 1));

				ImageFile* file = gImageFile.load();
				if (file)
				{
					video::ITexture* tex = driver->getTexture(file);
					if (tex)
					{
						active_buffer->Material.setTexture(0, tex);
					}
					file->drop();
				}

				scene::SMesh* mesh = new scene::SMesh;
				mesh->addMeshBuffer(active_buffer);
				mesh->recalculateBoundingBox();

				node = mgr->addMeshSceneNode(mesh);
				node->setRotation(core::vector3df(0, 45, 90));

				auto& c = node->getBoundingBox().getCenter();
				cam->setTarget(node->getPosition() + core::vector3df(0, c.X, -c.X));

				mesh->drop();
				active_buffer->drop();
			}

			if (device->run())
			{
				driver->beginScene(true, true, video::SColor(255, 128, 128, 128));
				mgr->drawAll();
				driver->endScene();
			}
			else
			{
				gViewMesh.store(nullptr);
				device->drop();
				gRunThread.clear();
				return;
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}

	int LoadModel(lua_State* L)
	{
		//vertices, triangles, decompressed textures
		luaL_checktype(L, 1, LUA_TTABLE);
		luaL_checktype(L, 2, LUA_TTABLE);
		int isDDS = lua_toboolean(L, 4);

		//find out if real PNGs should be non-flipped or what

		if (lua_istable(L, 3))
		{
			lua_getfield(L, 3, "ptr");
			byte* data = (byte*)lua_touserdata(L, -1);
			lua_getfield(L, 3, "inflated_len");
			uint32 len = lua_tointeger(L, -1);
			lua_getfield(L, 3, "png_name");
			const char* name = lua_tostring(L, -1);

			fipMemoryIO mem(data, len);
			fipImage img;
			img.clear();
			img.loadFromMemory(mem);
			fipMemoryIO out;
			img.saveToMemory(FIF_PNG, out);

			out.seek(0, SEEK_END);
			unsigned long size = out.tell();
			out.seek(0, SEEK_SET);
			byte* ptr = new byte[size];
			out.read(ptr, 1, size);

			gImageFile.store(new ImageFile(name, ptr, size));
		}
		else
		{
			gImageFile.store(nullptr);
		}

		scene::SMeshBuffer* mbuf = new scene::SMeshBuffer;
		auto& verts = mbuf->Vertices;
			
		int count = lua_objlen(L, 1);
		for (int i = 1; i <= count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, 1);

			video::S3DVertex vt;
			vt.Pos.X = Util::GetFloat(L, -1, "x");
			vt.Pos.Y = Util::GetFloat(L, -1, "y");
			vt.Pos.Z = Util::GetFloat(L, -1, "z");
			vt.Normal.X = Util::GetFloat(L, -1, "i");
			vt.Normal.Y = Util::GetFloat(L, -1, "j");
			vt.Normal.Z = Util::GetFloat(L, -1, "k");
			vt.TCoords.X = Util::GetFloat(L, -1, "u");
			float v = Util::GetFloat(L, -1, "v");
			if (isDDS)
				vt.TCoords.Y = (v > 0) ? -v : v;
			else
				vt.TCoords.Y = v;

			verts.push_back(vt);
			lua_pop(L, 1);
		}

		auto& idx = mbuf->Indices;

		count = lua_objlen(L, 2);
		for (int i = 1; i <= count; ++i)
		{
			lua_pushinteger(L, i);
			lua_gettable(L, 2);

			for (int j = 1; j <= 3; ++j)
			{
				lua_pushinteger(L, j);
				lua_gettable(L, -2);
				uint16 index = lua_tointeger(L, -1);
				idx.push_back(index);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}

		mbuf->recalculateBoundingBox();

		gViewMesh.store(mbuf);

		return 0;
	}

	int Open(lua_State* L)
	{
		//if flag is currently false and thread ptr has a value, it is deleteable
		if (gRunThread.test_and_set() == false && gViewerThread)
			delete gViewerThread;

		uint32 w = 0, h = 0;
		lua_getglobal(L, "settings");
		if (lua_istable(L, -1))
		{
			lua_getfield(L, -1, "viewer");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "width");
				w = lua_tointeger(L, -1);
				lua_pop(L, 1);
				lua_getfield(L, -1, "height");
				h = lua_tointeger(L, -1);
			}
		}

		gViewerThread = new std::thread(ThreadMain, (w >= 200) ? w : 500, (h >= 200) ? h : 500);
		gViewerThread->detach();

		return 0;
	}

	int Close(lua_State* L)
	{
		gRunThread.clear();
		return 0;
	}

	static const luaL_Reg funcs[] = {
		{"LoadModel", LoadModel},
		{"Open", Open},
		{"Close", Close},
		{nullptr, nullptr}
	};

	void LoadFunctions(lua_State* L)
	{
		luaL_register(L, "viewer", funcs);
	}
}
