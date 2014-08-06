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


#ifndef THIRDPARTY_CEF3_ZMQMONITOR_H
#define THIRDPARTY_CEF3_ZMQMONITOR_H

#include "zmq.hpp"
#include "tinythread.h"

class ZmqMonitor
{
public:
	ZmqMonitor(zmq::context_t& context, zmq::socket_t& socket);
	~ZmqMonitor();

	void start();
	void stop();

protected:
	static void runThread(void* pObj);
	void run();

private:
	volatile bool m_bIsStopped;
	tthread::thread* m_pWorkerThread;

	zmq::context_t& m_ZmqContext;
	zmq::socket_t& m_ZmqClient;
};


#endif