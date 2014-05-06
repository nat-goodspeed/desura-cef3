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

#include <Windows.h>
#include "SharedMem.h"

SharedMem::SharedMem()
	: m_hMappedFile(INVALID_HANDLE_VALUE)
	, m_nSize(0)
	, m_pMemory(NULL)
{
}

SharedMem::~SharedMem()
{
	if (m_pMemory)
		UnmapViewOfFile(m_pMemory);

	CloseHandle(m_hMappedFile);
}

bool SharedMem::init(const char* szName, size_t nSize, bool bReadOnly)
{
	if (!szName)
		return false;

	if (m_hMappedFile != INVALID_HANDLE_VALUE)
		return false;

	m_strName = std::string("Global\\") + szName;
	m_nSize = nSize;

	int nMemAccess = PAGE_READWRITE;
	int nFileAccess = FILE_MAP_ALL_ACCESS;

	if (bReadOnly)
	{
		nMemAccess = PAGE_READONLY;
		nFileAccess = FILE_MAP_READ;
	}

	m_hMappedFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, nMemAccess | SEC_COMMIT, 0, nSize, getName());

	if (!m_hMappedFile)
		return false;

	m_pMemory = MapViewOfFile(m_hMappedFile, nFileAccess, 0, 0, nSize);
	return !!m_pMemory;
}