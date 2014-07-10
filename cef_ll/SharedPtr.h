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

#ifndef THIRDPARY_CEF3_SHAREDPTR_H
#define THIRDPARY_CEF3_SHAREDPTR_H

#ifndef WIN32

namespace std
{
	template <typename T>
	class shared_ptr
	{
	public:
		shared_ptr(T* pT)
			: m_pT(pT)
			, m_nCount(new int(1))
		{
		}

		shared_ptr(const shared_ptr<T> &t)
			: m_pT(t.m_pT)
			, m_nCount(t.m_nCount)
		{
			++(*m_nCount);
		}

		~shared_ptr()
		{
			--(*m_nCount);
			if (*m_nCount == 0)
			{
				delete m_pT;
				delete m_nCount;
			}
		}

		T* operator->() const
		{
			return m_pT;
		}

		T* get() const
		{
			return m_pT;
		}

	private:
		int* m_nCount;
		T* m_pT;
	};

}
#endif

#endif
