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

#ifndef DESURA_SHAREDMEM_H
#define DESURA_SHAREDMEM_H
#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include <string>
#include "include/internal/cef_string.h"

/*****************************************************************************
*   [De]serialize to/from shared memory
*   TODO: Monty notes that the closest C++ equivalent to "passing a function
*   name" is passing pointer-to-member-function. We could wrap type-specific
*   functionality in classes, passing ptr-to-method to indicate read, write,
*   size.
*****************************************************************************/
/*--------------------------------- size_t ---------------------------------*/
union SizeToBuff
{
	volatile std::size_t sz;
	char b[sizeof(std::size_t)];
};

inline char* shm_size(char* szBuff, size_t)
{
	return szBuff + sizeof(std::size_t);
}

inline char* shm_write(char* szBuff, size_t sz)
{
	SizeToBuff u;
	u.sz = sz;
	memcpy(szBuff, u.b, sizeof(u.b));
	return szBuff + sizeof(u.b);
}

inline char* shm_read(char* szBuff, size_t& sz)
{
	SizeToBuff u;
	memcpy(u.b, szBuff, sizeof(u.b));
	sz = u.sz;
	return szBuff + sizeof(u.b);
}

/*------------------------------- CefString --------------------------------*/
// We optimize for the CefString case: both shm_write(std::string) and
// shm_write(CefString) store CefString data in shared memory, specifically so
// shm_read(CefString&) can construct a CefString which points directly into
// the shared-memory buffer instead of having to copy string data. (Naturally
// shm_read(std::string&) always copies the string data since std::string
// doesn't define any other behavior. It's up to the caller to read the type
// required by consuming code.)
inline char* shm_size(char* szBuff, const CefString& str)
{
	szBuff = shm_size(szBuff, str.size());
	// don't forget to account for the size of each character!
	return szBuff + (str.size() * sizeof(CefString::char_type));
}

inline char* shm_write(char* szBuff, const CefString& str)
{
	// Write the length as the number of CefString::char_type, not the total
	// byte length.
	szBuff = shm_write(szBuff, str.size());
	// But for copying data and skipping it, we need the byte length. Again,
	// account for the size of each character.
	size_t bytelen = str.size() * sizeof(CefString::char_type);
	memcpy(szBuff, str.c_str(), bytelen);
	return szBuff + bytelen;
}

inline char* shm_read(char* szBuff, CefString& str)
{
	std::size_t sz = 0;
	szBuff = shm_read(szBuff, sz);
	// Have to use reinterpret_cast<> because, although szBuff is a char*, we
	// happen to know it's pointing to a block of CefString::char_type.
	// DO NOT COPY the string data! (3rd param 'false')
	// Recall that sz is the number of CefString::char_type, not the total
	// number of bytes.
	str.FromString(reinterpret_cast<const CefString::char_type*>(szBuff), sz, false);
	// advance by the correct number of bytes
	return szBuff + (sz * sizeof(CefString::char_type));
}

/*------------------------------ std::string -------------------------------*/
// As noted above, std::string is converted to CefString for purposes of
// storing into shared memory. Specifically, if CefString::char_type !=
// std::string::value_type, the character data will be converted using
// CefString facilities before being copied to shared memory.
inline char* shm_size(char* szBuff, const std::string& str)
{
	// Formally, the correct thing to do would be to convert the incoming
	// std::string to CefString, which would certainly involve copying and
	// possibly character-width translation. Pragmatically, since we have to
	// do that work for real in shm_write() anyway, and since the only
	// CefString field used by shm_size(CefString) is its size(), we do
	// something a little bit evil here: we hand CefString's constructor a
	// bogus non-NULL pointer while forbidding it to copy the data.
	return shm_size(szBuff,
					CefString(reinterpret_cast<const CefString::char_type*>(str.c_str()),
							  str.size(),
							  false)); // no copy
}

inline char* shm_write(char* szBuff, const std::string& str)
{
	// Convert to CefString and write that. As noted in shm_size(), this means
	// copying and possibly character expansion.
	return shm_write(szBuff, CefString(str));
}

inline char* shm_read(char* szBuff, std::string& str)
{
	// Read into CefString...
	CefString temp;
	szBuff = shm_read(szBuff, temp);
	// then convert to string.
	str = temp.ToString();
	return szBuff;
}

/*--------------------------------- JSInfo ---------------------------------*/
struct JSInfo
{
	JSInfo() {}
	JSInfo(const CefString& name, const CefString& binding):
		strName(name),
		strBinding(binding)
	{}
	CefString strName;
	CefString strBinding;
};

inline char* shm_size(char* szBuff, const JSInfo& jsi)
{
	szBuff = shm_size(szBuff, jsi.strName);
	return	 shm_size(szBuff, jsi.strBinding);
}

inline char* shm_write(char* szBuff, const JSInfo& jsi)
{
	szBuff = shm_write(szBuff, jsi.strName);
	return	 shm_write(szBuff, jsi.strBinding);
}

inline char* shm_read(char* szBuff, JSInfo& jsi)
{
	JSInfo tmp;
	szBuff = shm_read(szBuff, tmp.strName);
	szBuff = shm_read(szBuff, tmp.strBinding);
	jsi = tmp;
	return szBuff;
}

/*----------------------------- std::vector<T> -----------------------------*/
template <typename T>
inline char* shm_size(char* szBuff, const std::vector<T>& vec)
{
	szBuff = shm_size(szBuff, vec.size());
	for (auto item : vec)
	{
		szBuff = shm_size(szBuff, item);
	}
	return szBuff;
}

template <typename T>
inline char* shm_write(char* szBuff, const std::vector<T>& vec)
{
	szBuff = shm_write(szBuff, vec.size());
	for (auto& item : vec)
	{
		szBuff = shm_write(szBuff, item);
	}
	return szBuff;
}

template <typename T>
inline char* shm_read(char* szBuff, std::vector<T>& vec)
{
	vec.clear();
	size_t count;
	szBuff = shm_read(szBuff, count);
	vec.resize(count);
	for (auto& item : vec)
	{
		szBuff = shm_read(szBuff, item);
	}
	return szBuff;
}

/*****************************************************************************
*   SharedMem
*****************************************************************************/
class SharedMem
{
public:
	SharedMem();
	~SharedMem();

	bool init(const char* szName, size_t nSize, bool bReadOnly = true);

	size_t getSize() const
	{
		return m_nSize;
	}

	void* getMem() const
	{
		return m_pMemory;
	}

	const char* getName() const
	{
		return m_strName.c_str();
	}

private:
#ifdef WIN32
	HANDLE m_hMappedFile;
#endif

	size_t m_nSize;
	std::string m_strName;

	void *m_pMemory;
};




#endif
