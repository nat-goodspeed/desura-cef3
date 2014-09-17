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

#include "ZmqMonitor.h"
#include "Controller.h"
#include <cassert>

ZmqMonitor::ZmqMonitor(zmq::context_t& context, zmq::socket_t& socket)
	: m_ZmqContext(context)
	, m_ZmqClient(socket)
	, m_bIsStopped(false)
	, m_pWorkerThread(NULL)
{
	zmq_socket_monitor(m_ZmqClient, "inproc://monitor.req", ZMQ_EVENT_ALL);
}

ZmqMonitor::~ZmqMonitor()
{
	stop();
	delete m_pWorkerThread;
}

void ZmqMonitor::start()
{
	if (m_pWorkerThread)
		return;

	m_pWorkerThread = new tthread::thread(&ZmqMonitor::runThread, this);
}

void ZmqMonitor::stop()
{
	m_bIsStopped = true;

	if (m_pWorkerThread && m_pWorkerThread->joinable())
		m_pWorkerThread->join();
}

void ZmqMonitor::runThread(void* pObj)
{
	try
	{
		static_cast<ZmqMonitor*>(pObj)->run();
	}
	catch (const std::exception& /*e*/)
	{
		int a = 1;
	}
}

void ZmqMonitor::run()
{
	zmq_event_t event;
	int rc;

	void *s = zmq_socket(m_ZmqContext, ZMQ_PAIR);
	assert(s);

	rc = zmq_connect(s, "inproc://monitor.req");
	assert(rc == 0);

	while (!m_bIsStopped)
	{
		zmq_msg_t msg;
		zmq_msg_init(&msg);
		rc = zmq_recvmsg(s, &msg, 0);

		if (rc == -1 && zmq_errno() == ETERM)
			break;

		assert(rc != -1);
		memcpy(&event, zmq_msg_data(&msg), sizeof(event));

		switch (event.event)
		{

		case ZMQ_EVENT_CONNECTED:
			cef3TraceS("ZMQ_EVENT_CONNECTED");
			break;

		case ZMQ_EVENT_CONNECT_DELAYED:
			cef3TraceS("ZMQ_EVENT_CONNECT_DELAYED");
			break;

		case ZMQ_EVENT_CONNECT_RETRIED:
			cef3TraceS("ZMQ_EVENT_CONNECT_RETRIED");
			break;

		case ZMQ_EVENT_LISTENING:
			cef3TraceS("ZMQ_EVENT_LISTENING");
			break;

		case ZMQ_EVENT_BIND_FAILED:
			cef3TraceS("ZMQ_EVENT_BIND_FAILED");
			break;

		case ZMQ_EVENT_ACCEPTED:
			cef3TraceS("ZMQ_EVENT_ACCEPTED");
			break;

		case ZMQ_EVENT_ACCEPT_FAILED:
			cef3TraceS("ZMQ_EVENT_ACCEPT_FAILED");
			break;

		case ZMQ_EVENT_CLOSED:
			cef3TraceS("ZMQ_EVENT_CLOSED");
			break;

		case ZMQ_EVENT_CLOSE_FAILED:
			cef3TraceS("ZMQ_EVENT_CLOSE_FAILED");
			break;

		case ZMQ_EVENT_DISCONNECTED:
			cef3TraceS("ZMQ_EVENT_DISCONNECTED");
			break;

		case ZMQ_EVENT_MONITOR_STOPPED:
			cef3TraceS("ZMQ_EVENT_MONITOR_STOPPED");
			m_bIsStopped = true;
			break;
		}
	}

	zmq_close(s);
}
