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

#ifndef WIN32

#include <sys/ipc.h>
#include <sys/shm.h>
#include "SharedMem.h"

SharedMem::SharedMem()
	: m_hMappedFile(-1)
	, m_nSize(0)
	, m_pMemory(NULL)
{
}

SharedMem::~SharedMem()
{
	if (m_pMemory)
		shmdt(m_pMemory);
}

bool SharedMem::init(const char* szName, size_t nSize, bool bReadOnly)
{
	if (!szName)
		return false;

	if (m_hMappedFile != -1)
		return false;

	m_strName = std::string("Global\\") + szName;
	m_nSize = nSize;

	int nKey = 0;
	m_hMappedFile = shmget(nKey, nSize, IPC_CREAT | 0666);

	if (m_hMappedFile == -1)
		return false;

	m_pMemory = shmat(m_hMappedFile, NULL, bReadOnly?SHM_RDONLY:0);
	return m_pMemory != (char*)-1;
}

#endif
