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

#include "ChromiumBrowserI.h"
#include "SharedObjectLoader.h"


#ifdef WIN32

#include <Windows.h>

typedef BOOL(WINAPI* SetDllDirectoryFunc)(LPCTSTR lpPathName);

bool SetDllDir(const char* dir)
{
	SharedObjectLoader sol;

	if (sol.load("kernel32.dll"))
	{
		SetDllDirectoryFunc set_dll_directory = sol.getFunction<SetDllDirectoryFunc>("SetDllDirectoryA");

		if (set_dll_directory && set_dll_directory(dir))
			return true;
	}

	return false;
}

typedef int(*CEF_ExecuteProcessFn)(HINSTANCE);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef DEBUG
	//while (!IsDebuggerPresent())
	//	Sleep(1000);
#endif

	if (!SetDllDir(".\\bin"))
		return -1;

	SharedObjectLoader sol;

	if (!sol.load("3p_cef3.dll"))
		return -2;

	CEF_ExecuteProcessFn CEF_ExecuteProcess = sol.getFunction<CEF_ExecuteProcessFn>("CEF_ExecuteProcessWin");

	if (!CEF_ExecuteProcess)
		return -3;

	return CEF_ExecuteProcess(hInstance);
}

#else

typedef int (*CEF_ExecuteProcessFn)(int, char**);

int main(int argc, char** argv)
{
	SharedObjectLoader sol;

	if (!sol.load("3p_cef3.dll"))
		return -1;

	CEF_ExecuteProcessFn CEF_ExecuteProcess = sol.getFunction<CEF_ExecuteProcessFn>("CEF_ExecuteProcess");

	if (!CEF_ExecuteProcess)
		return -2;

	return CEF_ExecuteProcess(argc, argv);
}

#endif



