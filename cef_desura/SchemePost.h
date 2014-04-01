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

#ifndef DESURA_SCHEMEPOST_H
#define DESURA_SCHEMEPOST_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
//#include "include/cef.h"
#include "include/cef_request.h"
#include "include/internal/cef_ptr.h"

class PostElement : public ChromiumDLL::PostElementI
{
public:
	PostElement();
	PostElement(CefRefPtr<CefPostDataElement> element);

	//! Deletes the object. Should never be called by user code!
	//!
	virtual void destroy()
	{
		delete this;
	}

	virtual bool isFile();
	virtual bool isBytes();

	virtual void setToEmpty();
	virtual void setToFile(const char* fileName);
	virtual void setToBytes(size_t size, const void* bytes);

	virtual void getFile(char *buff, size_t buffsize);

	virtual size_t getBytesCount();
	virtual size_t getBytes(size_t size, void* bytes);

	CefRefPtr<CefPostDataElement> getHandle()
	{
		return m_rPostElement;
	}

private:
	CefRefPtr<CefPostDataElement> m_rPostElement;
};


class PostData : public ChromiumDLL::PostDataI
{
public:
	PostData();
	PostData(CefRefPtr<CefPostData> data);


	//! Deletes the object. Should never be called by user code!
	//!
	virtual void destroy()
	{
		delete this;
	}

	virtual size_t getElementCount();
	virtual ChromiumDLL::PostElementI* getElement(size_t index);

	virtual bool removeElement(ChromiumDLL::PostElementI* element);
	virtual bool addElement(ChromiumDLL::PostElementI* element);

	virtual void removeElements();

	CefRefPtr<CefPostData> getHandle()
	{
		return m_rPostData;
	}

private:
	CefRefPtr<CefPostData> m_rPostData;
};





#endif //DESURA_SCHEMEPOST_H
