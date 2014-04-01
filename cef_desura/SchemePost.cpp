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

#include "SchemePost.h"
#include <string>

#define _CRT_SECURE_NO_WARNINGS

int mystrncpy_s(char* dest, size_t destSize, const char* src, size_t srcSize);

PostElement::PostElement()
{
}

PostElement::PostElement(CefRefPtr<CefPostDataElement> element)
{
	m_rPostElement = element;
}

bool PostElement::isFile()
{
	return m_rPostElement->GetType() == PDE_TYPE_BYTES;
}

bool PostElement::isBytes()
{
	return m_rPostElement->GetType() == PDE_TYPE_FILE;
}

void PostElement::setToEmpty()
{
	m_rPostElement->SetToEmpty();
}

void PostElement::setToFile(const char* fileName)
{
	m_rPostElement->SetToFile(fileName);
}

void PostElement::setToBytes(size_t size, const void* bytes)
{
	m_rPostElement->SetToBytes(size, bytes);
}

void PostElement::getFile(char *buff, size_t buffsize)
{
	std::string file = m_rPostElement->GetFile();

	if (buff)
		mystrncpy_s(buff, buffsize, file.c_str(), file.size());
}

size_t PostElement::getBytesCount()
{
	return m_rPostElement->GetBytesCount();
}

size_t PostElement::getBytes(size_t size, void* bytes)
{
	return m_rPostElement->GetBytes(size, bytes);
}






PostData::PostData()
{

}

PostData::PostData(CefRefPtr<CefPostData> data)
{
	m_rPostData = data;
}

size_t PostData::getElementCount()
{
	return m_rPostData->GetElementCount();
}

ChromiumDLL::PostElementI* PostData::getElement(size_t index)
{
	CefPostData::ElementVector eles;
	m_rPostData->GetElements(eles);

	if (index >= eles.size())
		return NULL;

	return new PostElement(eles[index]);
}

bool PostData::removeElement(ChromiumDLL::PostElementI* element)
{
	PostElement *pe = (PostElement*)element;

	bool res = false;

	if (pe)
		res = m_rPostData->RemoveElement(pe->getHandle());

	element->destroy();

	return res;
}

bool PostData::addElement(ChromiumDLL::PostElementI* element)
{
	PostElement *pe = (PostElement*)element;

	bool res = false;

	if (pe)
		res = m_rPostData->AddElement(pe->getHandle());

	element->destroy();

	return res;
}

void PostData::removeElements()
{
	m_rPostData->RemoveElements();
}
