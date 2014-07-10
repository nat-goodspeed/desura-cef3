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

#include <string>
#include <string.h>

union IntToBuff
{
	int i;
	char b[4];
};

inline void writeInt(char* szBuff, int nVal)
{
	IntToBuff t;
	t.i = nVal;
	memcpy(szBuff, t.b, 4);
}

inline int readInt(char* szBuff)
{
	IntToBuff t;
	memcpy(t.b, szBuff, 4);

	return t.i;
}

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
#else
	int m_hMappedFile;
#endif

	size_t m_nSize;
	std::string m_strName;

	void *m_pMemory;
};




#endif
