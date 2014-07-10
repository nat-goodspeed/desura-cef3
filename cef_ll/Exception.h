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

#ifndef THIRDPARY_CEF3_EXCEPTION_H
#define THIRDPARY_CEF3_EXCEPTION_H

#ifdef WIN32

typedef std::exception exception;

#else

#include <string>

class exception : public std::exception
{
public:
	exception(const std::string& strWhat)
		: m_strWhat(strWhat)
	{
	}

	virtual ~exception() _GLIBCXX_USE_NOEXCEPT
	{
	}

	const char* what() const _GLIBCXX_USE_NOEXCEPT
	{
		return m_strWhat.c_str();
	}
	
private:
	const std::string m_strWhat;
};

#endif

#endif
