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

#ifndef DESURA_SHAREDOBJECTLOADER_H
#define DESURA_SHAREDOBJECTLOADER_H
#ifdef _WIN32
#pragma once
#endif

#ifdef WIN32
	#include <Windows.h>
#else
    #include <dlfcn.h>
#endif

#ifdef NIX
#define OS_LINUX
#endif

class SharedObjectLoader
{
public:
#ifdef OS_LINUX
    typedef void* SOHANDLE;
#else
	typedef HINSTANCE SOHANDLE;
#endif

	SharedObjectLoader()
	{
		m_hHandle = NULL;
		m_bHasFailed = false;
	}

	SharedObjectLoader(const SharedObjectLoader& sol)
	{
		m_hHandle = sol.m_hHandle;
		m_bHasFailed = sol.m_bHasFailed;

		sol.m_hHandle = NULL;
		sol.m_bHasFailed = false;
	}

	~SharedObjectLoader()
	{
		unload();
	}

	bool load(const char* module)
	{
		if (m_hHandle)
			unload();

		m_bHasFailed = false;

#ifdef OS_LINUX
		m_hHandle = dlopen(module, RTLD_LAZY);
#else
		m_hHandle = LoadLibraryA(module);
#endif
		return m_hHandle?true:false;
	}

	void unload()
	{
		if (!m_hHandle)
			return;

#ifdef NIX
		dlclose(m_hHandle);
#else
		FreeLibrary(m_hHandle);
#endif

		m_hHandle = NULL;
	}

	template <typename T>
	T getFunction(const char* functionName)
	{
		if (!m_hHandle)
			return NULL;
#ifdef NIX
		T fun = (T)dlsym(m_hHandle, functionName);
#else
		T fun = (T)GetProcAddress(m_hHandle, functionName);
#endif

		if (!fun)
			m_bHasFailed = true;

		return fun;
	}

	bool hasFailed()
	{
		return m_bHasFailed;
	}

	SOHANDLE handle()
	{
		return m_hHandle;
	}

private:
	mutable bool m_bHasFailed;
#ifdef WIN32
	mutable SOHANDLE m_hHandle;
#else
	mutable void* m_hHandle;
#endif
};

#endif //DESURA_SHAREDOBJECTLOADER_H
