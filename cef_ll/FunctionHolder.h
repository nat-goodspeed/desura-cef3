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


#ifndef DESURA_FUNCTIONHOLDER_H
#define DESURA_FUNCTIONHOLDER_H


#include "tinythread.h"


template <typename T, char TOKEN>
class FunctionHolder
{
public:
	FunctionHolder()
		: m_nLastId(0)
	{
	}

	std::string newKey()
	{
		tthread::lock_guard<tthread::mutex> guard(m_Lock);

		++m_nLastId;
		char szId[255] = { 0 };

#ifdef WIN32
		_snprintf(szId, 255, "%c:%d", TOKEN, m_nLastId);
#else
		assert(false);
#endif
		return szId;
	}

	void add(const std::string &strId, const T &funct)
	{
		tthread::lock_guard<tthread::mutex> guard(m_Lock);
		m_mFunctionMap[strId] = funct;
	}

	void del(const std::vector<std::string> &vIds)
	{
		tthread::lock_guard<tthread::mutex> guard(m_Lock);

		for (size_t x = 0; x<vIds.size(); ++x)
			internalDelete(vIds[x]);
	}

	bool del(const std::string &strId)
	{
		tthread::lock_guard<tthread::mutex> guard(m_Lock);
		return internalDelete(strId);
	}

	T find(const std::string &strId) const
	{
		tthread::lock_guard<tthread::mutex> guard(m_Lock);

		std::map<std::string, T>::const_iterator it = m_mFunctionMap.find(strId);

		if (it != m_mFunctionMap.cend())
			return it->second;

		return NULL;
	}

	std::map<std::string, T> duplicate() const
	{
		tthread::lock_guard<tthread::mutex> guard(m_Lock);
		return m_mFunctionMap;
	}

	bool isRendererFunction(const std::string &strId) const
	{
		return strId.find("R:") == 0;
	}

	bool isBrowserFunction(const std::string &strId) const
	{
		return strId.find("B:") == 0;
	}

protected:
	bool internalDelete(const std::string &strId)
	{
		std::map<std::string, T>::iterator it = m_mFunctionMap.find(strId);

		if (it == m_mFunctionMap.end())
			return false;

		m_mFunctionMap.erase(it);
		return true;
	}

private:
	mutable tthread::mutex m_Lock;

	int m_nLastId;
	std::map<std::string, T> m_mFunctionMap;
};

#endif