
#include <cstdio>
#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <lua.hpp>
#include <irrlicht.h>
#include <FreeImagePlus.h>
#include "types.h"
#include "util.h"

using namespace irr;

namespace Viewer
{
	void LoadFunctions(lua_State* L);

	class ImageFile : public irr::io::IReadFile
	{
	public:
		ImageFile(const char* name, byte* data, uint32 len) :
			mName(name), mData(data), mLength(len), mPos(0)
		{

		}

		~ImageFile()
		{
			if (mData)
				delete[] mData;
		}

		virtual const io::path& getFileName() const
		{
			return mName;
		}

		virtual long getPos() const
		{
			return mPos;
		}

		virtual long getSize() const
		{
			return mLength;
		}

		virtual int32 read(void* buffer, uint32 sizeToRead)
		{
			long read = mPos + sizeToRead;
			if (read >= mLength)
				sizeToRead = mLength - mPos;
			memcpy(buffer, &mData[mPos], sizeToRead);
			mPos = read;
			return sizeToRead;
		}

		virtual bool seek(long finalPos, bool relativeMovement = false)
		{
			if (relativeMovement)
				finalPos += mPos;
			if (finalPos >= mLength)
				return false;
			mPos = finalPos;
			return true;
		}

	private:
		irr::io::path mName;
		byte* mData;
		long mLength;
		long mPos;
	};
}
