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


#ifdef WIN32
#include <windows.h>
#include "Tracer.h"
#include <sstream>

TracerStorage::TracerStorage(const wchar_t* szSharedMemName, bool bFirst)
	: m_szSharedMemName(szSharedMemName)
{
	uint32_t nSize = getTotalSize() + 12;

	m_hMappedFile = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE|SEC_COMMIT, 0, nSize, getSharedMemName());

	if (!m_hMappedFile)
		return;

	m_pHeader = (TracerHeader_s*)MapViewOfFile(m_hMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, nSize);

	m_nCurLock = &m_pHeader->lock;
	m_szMappedMemory = &m_pHeader->data;

	if (bFirst)
	{
		m_pHeader->pid = GetCurrentProcessId();
		m_pHeader->segCount = m_nNumSegments;
		m_pHeader->segSize = m_nSegmentSize;

		for (uint32_t x = 0; x < nSize; x += m_nSegmentSize)
			(m_szMappedMemory + x)[0] = 0;
	}
}

TracerStorage::~TracerStorage()
{
	if (m_pHeader)
		UnmapViewOfFile(m_pHeader);

	CloseHandle(m_hMappedFile);
}

void TracerStorage::trace(const std::string &strTrace, std::map<std::string, std::string> *pmArgs)
{
	if (!m_szMappedMemory || !m_nCurLock)
		return;

	std::string strFormated = formatTrace(strTrace, pmArgs);
	uint16_t pos = InterlockedIncrement(m_nCurLock) % m_nNumSegments;

	if (pos == 0)
		pos = m_nNumSegments;

	auto saveSpot = m_szMappedMemory + (m_nSegmentSize * (pos - 1));
	strncpy_s(saveSpot, m_nSegmentSize, strFormated.c_str(), strFormated.size());
}

const wchar_t* TracerStorage::getSharedMemName()
{
	return m_szSharedMemName;
}

uint32_t TracerStorage::getTotalSize() const
{
	return m_nNumSegments * m_nSegmentSize;
}

std::string TracerStorage::formatTrace(const std::string &szMessage, std::map<std::string, std::string> *pmArgs)
{
	std::ostringstream ss;
	ss << "{ \"message\": \"" << cleanUpString(szMessage) << "\"";

	if (pmArgs)
	{
		for (auto p : *pmArgs)
		{
			if (p.second.empty())
				continue;

			auto sec = cleanUpString(p.second);

			std::ostringstream temp;
			temp << ", \"" << p.first << "\": \"" << sec << "\"";

			auto t = temp.str();

			if ((uint32_t)ss.tellp() + t.size() + 2 > m_nSegmentSize)
				break;

			ss << t;
		}
	}

	ss << " }";
	auto ret = ss.str();
	return ret;
}

std::string TracerStorage::cleanUpString(const std::string &string)
{
	std::string out;
	out.reserve(string.size() + 20);

	for (auto c : string)
	{
		if (c == '"' || c == '\\')
			out += '\\';

		if (c == '\n')
			out += "\\n";
		else if (c == '\t')
			out += "\\t";
		else
			out += c;
	}

	return out;
}


#endif