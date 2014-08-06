/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

$LicenseInfo:firstyear=2014&license=lgpl$
Copyright (C) 2014, Linden Research, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
version 2.1 of the License only.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>
or write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
$/LicenseInfo$
*/


#ifndef THIRDPARTY_CEF3_REFCOUNT_HEADER
#define THIRDPARTY_CEF3_REFCOUNT_HEADER
#ifdef _WIN32
#pragma once
#endif

#include <assert.h>

namespace ChromiumDLL
{

#ifdef WIN32
	template <typename T>
	class ChromiumRefCount : public T
	{
	public:
		ChromiumRefCount()
			: m_RefCount(0)
		{
		}

		~ChromiumRefCount()
		{
			assert(m_RefCount == 0);
		}

		void addRef()
		{
			InterlockedIncrement(&m_RefCount);
		}

		void delRef()
		{
			if (InterlockedDecrement(&m_RefCount) == 0)
				destroy();
		}

	private:
		LONG m_RefCount;
	};
#else
	template <typename T>
	class ChromiumRefCount : public T
	{
	public:
		ChromiumRefCount()
		{
			//TODO
			assert(false);
		}

		~ChromiumRefCount()
		{
			//TODO
			assert(false);
		}

		void addRef()
		{
			//TODO
			assert(false);
		}

		void delRef()
		{
			//TODO
			assert(false);
		}
};
#endif

}


#endif