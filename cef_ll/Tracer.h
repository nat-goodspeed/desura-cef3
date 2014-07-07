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

#ifndef DESURA_TRACER_H
#define DESURA_TRACER_H

#include <list>
#include <map>
#include <stdint.h>

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	uint32_t pid;
	uint32_t lock;
	uint16_t segCount;
	uint16_t segSize;
	char data;
} TracerHeader_s;

#pragma pack(pop)


#ifdef WIN32

class TracerStorage
{
public:
	TracerStorage(const wchar_t* szSharedMemName, bool bFirst);
	~TracerStorage();

	void trace(const std::string &strTrace, std::map<std::string, std::string> *mpArgs);

	const wchar_t* getSharedMemName();

protected:
	uint32_t getTotalSize() const;

	std::string formatTrace(const std::string &strTrace, std::map<std::string, std::string> *mpArgs);
	std::string cleanUpString(const std::string &string);

private:
	//this number of segments should cause perfect roll around
	const uint16_t m_nNumSegments = 4096;
	const uint16_t m_nSegmentSize = 512;

	volatile uint32_t* m_nCurLock = nullptr;

	HANDLE m_hMappedFile = INVALID_HANDLE_VALUE;
	char* m_szMappedMemory = nullptr;

	TracerHeader_s* m_pHeader = nullptr;
	const wchar_t* m_szSharedMemName = nullptr;
};

#else

class TracerStorage
{
public:
	TracerStorage(const wchar_t* szSharedMemName, bool bFirst)
	{
	}

	void trace(const std::string &strTrace, std::map<std::string, std::string> *mpArgs) override
	{
	}
};

#endif

#endif