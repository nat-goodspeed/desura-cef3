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

#include <gtk/gtk.h>
#include "ChromiumBrowserI.h"
#include "SharedObjectLoader.h"

ChromiumDLL::ChromiumControllerI* g_ChromiumController = NULL;
ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserI> g_Browser;

typedef ChromiumDLL::ChromiumControllerI* (*CEF_InitFn)(bool, const char*, const char*, const char*);

static gboolean HandleFocus(GtkWidget* widget, GdkEventFocus* focus)
{
	if (g_Browser)
		g_Browser->onFocus();
}

void destroy(GtkWidget* widget, gpointer data) 
{
}

gboolean delete_event(GtkWidget* widget, GdkEvent* event, GtkWindow* window) 
{
	g_Browser = NULL;
}

int main(int argc, char** argv)
{
	gtk_init(&argc, &argv);

	SharedObjectLoader sol;

	if (!sol.load("libcef_desura.so"))
		return -1;

	CEF_InitFn CEF_Init = sol.getFunction<CEF_InitFn>("CEF_InitEx");

	if (!CEF_Init)
		return -2;

	g_ChromiumController = CEF_Init(false, "cache", "log", "UserAgent");

	if (!g_ChromiumController)
		return -3;

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);

	g_signal_connect(window, "focus", G_CALLBACK(&HandleFocus), NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event), window);

	g_Browser = g_ChromiumController->NewChromiumBrowser((int*)vbox, "", "http://google.com");

	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(GTK_WIDGET(window));

	g_ChromiumController->RunMsgLoop();
	g_ChromiumController->Stop();
	return 0;
}
