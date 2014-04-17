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

#import <Cocoa/Cocoa.h>
#include "ChromiumBrowserI.h"
#include "SharedObjectLoader.h"

ChromiumDLL::ChromiumControllerI* g_ChromiumController = NULL;
ChromiumDLL::ChromiumBrowserI* g_Browser = NULL;
NSView* g_ContentView = NULL;


typedef ChromiumDLL::ChromiumControllerI* (*CEF_InitFn)(bool, const char*, const char*, const char*);

#import <AppKit/AppKit.h>

// Copy of CrAppProtocol definition from base/message_pump_mac.h.
@protocol CrAppProtocol
// Must return true if -[NSApplication sendEvent:] is currently on the stack.
- (BOOL)isHandlingSendEvent;
@end

@protocol CefAppProtocol<CrAppProtocol>
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;
@end

// Controls the state of |isHandlingSendEvent| in the event loop so that it is
// reset properly.
class CefScopedSendingEvent {
public:
  CefScopedSendingEvent()
    : app_(static_cast<NSApplication<CefAppProtocol>*>(
              [NSApplication sharedApplication])),
      handling_([app_ isHandlingSendEvent]) {
    [app_ setHandlingSendEvent:YES];
  }
  ~CefScopedSendingEvent() {
    [app_ setHandlingSendEvent:handling_];
  }
  
private:
  NSApplication<CefAppProtocol>* app_;
  BOOL handling_;
};



// Provide the CefAppProtocol implementation required by CEF.
@interface SimpleApplication : NSApplication<CefAppProtocol> {
@private
  BOOL handlingSendEvent_;
}
@end

@implementation SimpleApplication
- (BOOL)isHandlingSendEvent {
  return handlingSendEvent_;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handlingSendEvent_ = handlingSendEvent;
}

- (void)sendEvent:(NSEvent*)event {
  CefScopedSendingEvent sendingEventScoper;
  [super sendEvent:event];
}
@end





// Receives notifications from the application.
@interface SimpleAppDelegate : NSObject
- (void)createApp:(id)object;
@end

@implementation SimpleAppDelegate

// Create the application on the UI thread.
- (void)createApp:(id)object 
{
	[NSApplication sharedApplication];
	[NSBundle loadNibNamed:@"MainMenu" owner:NSApp];

	// Set the delegate for application events.
	[NSApp setDelegate:self];


	float kWindowHeight =0.75;
    float kWindowWidth = 0.75;
	// Create the main application window.
	NSRect screen_rect = [[NSScreen mainScreen] visibleFrame];
	NSRect window_rect = { {0, screen_rect.size.height - kWindowHeight},
	{kWindowWidth, kWindowHeight} };
	
	NSWindow* mainWnd = [[NSWindow alloc]//[UnderlayOpenGLHostingWindow alloc]
					   initWithContentRect:window_rect
					   styleMask:(NSTitledWindowMask |
								  NSClosableWindowMask |
								  NSMiniaturizableWindowMask |
								  NSResizableWindowMask )
					   backing:NSBackingStoreBuffered
					   defer:NO];
					   
	[mainWnd setTitle:@"cefclient"];
	[mainWnd setDelegate:self];
	
	// Rely on the window delegate to clean us up rather than immediately
	// releasing when the window gets closed. We use the delegate to do
	// everything from the autorelease pool so the window isn't on the stack
	// during cleanup (ie, a window close from javascript).
	[mainWnd setReleasedWhenClosed:NO];
	
	g_ContentView = [mainWnd contentView];
	
	// Show the window.
	[mainWnd makeKeyAndOrderFront: nil];
}

// Called when the application's Quit menu item is selected.
- (NSApplicationTerminateReply)applicationShouldTerminate:
      (NSApplication *)sender 
{
	// Request that all browser windows close.
	//if (g_Browser)
		////g_Browser->Close();

	// Cancel the termination. The application will exit after all windows have
	// closed.
	return NSTerminateCancel;
}

// Sent immediately before the application terminates. This signal should not
// be called because we cancel the termination.
- (void)applicationWillTerminate:(NSNotification *)aNotification {
  //ASSERT(false);  // Not reached.
}

@end



int main(int argc, char* argv[]) 
{
	SharedObjectLoader sol;

	if (!sol.load("libcef_desura.dylib"))
		return -1;

	CEF_InitFn CEF_Init = sol.getFunction<CEF_InitFn>("CEF_InitEx");

	if (!CEF_Init)
		return -2;

	g_ChromiumController = CEF_Init(false, "cache", "log", "UserAgent");

	if (!g_ChromiumController)
		return -3;

	// Initialize the AutoRelease pool.
	NSAutoreleasePool* autopool = [[NSAutoreleasePool alloc] init];

	// Initialize the SimpleApplication instance.
	[SimpleApplication sharedApplication];

	// Create the application delegate.
	NSObject* delegate = [[SimpleAppDelegate alloc] init];
	[delegate performSelectorOnMainThread:@selector(createApp:) withObject:nil
						  waitUntilDone:NO];

	g_Browser = g_ChromiumController->NewChromiumBrowser((int*)g_ContentView, "", "http://google.com");

	g_ChromiumController->RunMsgLoop();
	g_ChromiumController->Stop();

	// Release the delegate.
	[delegate release];

	// Release the AutoRelease pool.
	[autopool release];

	return 0;
}
