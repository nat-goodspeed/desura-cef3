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

#include <iostream>

#ifdef _WIN32
    #include "freeglut/freeglut.h"
#else
    #include <GL/gl.h>
    #include <GL/glut.h>
#endif

#include <string.h>
#include <stdio.h>

#define CEF_IGNORE_FUNCTIONS 1
#include "ChromiumBrowserI.h"
#include "ChromiumWin32KeyPress.h"
#include "ChromiumRefCount.h"
#include "SharedObjectLoader.h"

#include "ll_jsbridge_test.h"

#ifndef WIN32
#define override
#endif

#if defined(NIX)
#include <pthread.h>

class Mutex
{
public:
	Mutex()
	{
		pthread_mutex_init(&m_Mutex, NULL);
	}

	~Mutex()
	{
		pthread_mutex_destroy(&m_Mutex);
	}

	bool lock()
	{
		return pthread_mutex_lock(&m_Mutex);
	}

	bool unlock()
	{
		return pthread_mutex_unlock(&m_Mutex);
	}

private:
	pthread_mutex_t m_Mutex;
};

#elif defined(WIN32)
#include <windows.h>
#include <process.h>

class Mutex
{
public:
	Mutex()
	{
		m_Mutex = CreateMutex(NULL, FALSE, NULL);
	}

	~Mutex()
	{
		CloseHandle(m_Mutex);
	}

	bool lock()
	{
		return WaitForSingleObject(m_Mutex, INFINITE) == WAIT_FAILED;
	}

	bool unlock()
	{
		return ReleaseMutex(m_Mutex) == 0;
	}

private:
	HANDLE m_Mutex;
};
#endif


class LockGaurd
{
public:
	LockGaurd(Mutex &mutex)
		: m_Mutex(mutex)
	{
		m_Mutex.lock();
	}

	~LockGaurd()
	{
		m_Mutex.unlock();
	}

private:
	Mutex &m_Mutex;
};

class Proxy : public ChromiumDLL::ChromiumBrowserEventI_V2, public ChromiumDLL::ChromiumRendererEventI, public ChromiumDLL::ChromiumRendererPopupEventI
{
public:
	void destroy()
	{
		delete this;
	}
};

class cefGL : 
	public ChromiumDLL::ChromiumRefCount<Proxy>,
	public ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>
{
    public:
        cefGL() :
			// TODO make this work with non power of two sizes
            mAppWindowWidth( 1024 )                     // dimensions of the app window - can be anything
			, mAppWindowHeight(1024)
			, mAppTextureWidth(-1)                     // dimensions of the texture that the browser is rendered into
			, mAppTextureHeight(-1)                   // calculated at initialization

			, mAppTexture(0)                           // OpenGL texture handle
			, mAppTexturePixels(NULL)

			, mAppWindowName("cefGL")
			, mNeedsUpdate(true)                       // flag to indicate if browser texture needs an update
			, mNeedsResize(true)

			, pRenderer(NULL)

#ifdef WIN32
			, m_hMenu(NULL)
#endif
        {
			//std::cout << "LLQtWebKit version: " << LLQtWebKit::getInstance()->getVersion() << std::endl;
        };

		bool enablePlugins()
		{
			return false;
		}

		bool enableFlash()
		{
			return false;
		}

		void initOpenGL()
		{
            glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
            glEnable( GL_COLOR_MATERIAL );
            glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
            glEnable( GL_TEXTURE_2D );
            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glEnable( GL_CULL_FACE );


            // create the texture used to display the browser data
            glGenTextures( 1, &mAppTexture );
            glBindTexture( GL_TEXTURE_2D, mAppTexture );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, mAppTextureWidth, mAppTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
		}

		void initCEF()
		{
#ifdef WIN32
			const char* szCefDLL = "3p_cef3.dll";
#else
			const char* szCefDLL = "lib3p_cef3.dll";
#endif

			if (!g_CEFDll.load(szCefDLL))
			{
#ifdef WIN32
				printf("Failed to load cef library: {0}\n", GetLastError());
#else
				printf("Failed to load cef library\n");
#endif
				exit(1);
			}
			std::cout << "Loaded CEF library successfully" << std::endl;

			CEF_Init = g_CEFDll.getFunction<CEF_InitFn>("CEF_InitEx");

			if (g_CEFDll.hasFailed())
			{
				printf("Failed to find cef library exports\n");
				exit(1);
			}
            std::cout << "Found CEF library exports" << std::endl;

#ifdef WIN32
			pController = CEF_Init(true, "CachePath", "LogPath", "UserAgent");
#else
			pController = CEF_Init(false, "CachePath", "LogPath", "UserAgent");
#endif

			if ( ! pController  )
			{
				std::cout << "Failed to initialize CEF" << std::endl;
				exit(1);
			}
			std::cout << "CEF initialized successfully" << std::endl;

			pController->SetApiVersion(2);
			pController->RegisterSchemeExtender(new JSBridgeTestScheme());
			pController->RegisterJSExtender(new JSBridgeTestExtender(pController));

#ifdef WIN32
			m_hWnd = FindWindow(NULL, mAppWindowName.c_str());

			pRenderer = pController->NewChromiumRenderer((int*)m_hWnd, "jsbridge://run", mAppTextureWidth, mAppTextureHeight);
			g_pRenderer = pRenderer;

			HHOOK hook = SetWindowsHookEx(WH_GETMESSAGE, handleWinMsg, GetModuleHandle(NULL), GetWindowThreadProcessId(m_hWnd, 0));
#else
			pRenderer = pController->NewChromiumRenderer((int*)NULL, "jsbridge://run", mAppTextureWidth, mAppTextureHeight);
#endif

			if ( ! pRenderer  )
			{
				std::cout << "Failed to create and initialize CEF renderer" << std::endl;
				exit(1);
			}
			std::cout << "CEF renderer created and initialized successfully" << std::endl;

			pRenderer->getBrowser()->setEventCallback(this);

			pRenderer->setEventCallback(ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererEventI>(this));
			pRenderer->setEventCallback(ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererPopupEventI>(this));
		}

#ifdef WIN32
		static ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererI> g_pRenderer;

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

		static LRESULT CALLBACK handleWinMsg(int nCode, WPARAM wParam, LPARAM lParam)
		{
			if (nCode != HC_ACTION || wParam != PM_REMOVE || !g_pRenderer)
				return CallNextHookEx(0, nCode, wParam, lParam);

			MSG *pMsg = (MSG*)lParam;

			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);

			switch (pMsg->message)
			{
			case WM_SYSCHAR:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_CHAR:
				g_pRenderer->onKeyPress(new ChromiumDLL::Win32ChromiumKeyPress(pMsg->message, pMsg->wParam, pMsg->lParam));
				break;

			case WM_MOUSEWHEEL:
				//(*g_pRenderer)->getBrowser()->scroll(GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam), GET_WHEEL_DELTA_WPARAM(pMsg->wParam), 0);
				break;

			case WM_MOUSEMOVE:
				break;
			}

			return CallNextHookEx(0, nCode, wParam, lParam);
		}
#endif

		void idle()
		{
#ifdef NIX
			if (pController)
				pController->DoMsgLoop();
#endif
#ifdef WIN32
			if (m_hMenu)
			{
				int screen_pos_x = glutGet((GLenum)GLUT_WINDOW_X);
				int screen_pos_y = glutGet((GLenum)GLUT_WINDOW_Y);

				TrackPopupMenu(m_hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_RECURSE | TPM_NONOTIFY, m_nLastXIn + screen_pos_x, m_nLastYIn + screen_pos_y, 0, m_hWnd, NULL);
				DestroyMenu(m_hMenu);
				m_hMenu = NULL;
			}
				
#endif
        }

        void init()
        {
			initCEF();
			initOpenGL();
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        void reset( void )
        {
			pController->Stop();
			//delete [] mAppTexturePixels;
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        void reshape( int widthIn, int heightIn )
        {
            std::cout << "reshape " <<  widthIn << " x " << heightIn << std::endl;

			if ( heightIn == 0 )
				heightIn = 1;

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();

			glViewport( 0, 0, widthIn, heightIn );
			glOrtho( 0.0f, widthIn, heightIn, 0.0f, -1.0f, 1.0f );

			mAppWindowWidth = widthIn;
			mAppWindowHeight = heightIn;

			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			mNeedsUpdate = true;

			glutPostRedisplay();	

			if (pRenderer)
				pRenderer->invalidateSize();
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        void display()
        {
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			glBindTexture(GL_TEXTURE_2D, mAppTexture);

			if (mNeedsUpdate && mAppTexturePixels)
			{
				LockGaurd guard(m_BufferLock);
				
				if (mNeedsResize)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mAppTextureWidth, mAppTextureHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, mAppTexturePixels);
				else
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mAppTextureWidth, mAppTextureHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, mAppTexturePixels);

				mNeedsResize = false;
				mNeedsUpdate = false;
			}

            // scale the texture so that it fits the screen
			// TODO: move this to reshape method
			GLfloat textureScaleX = (GLfloat)mAppTextureWidth / (GLfloat)mAppTextureWidth;
			GLfloat textureScaleY = (GLfloat)mAppTextureHeight / (GLfloat)mAppTextureHeight;

            // draw the single quad full screen (orthographic)
            glMatrixMode( GL_TEXTURE );
            glPushMatrix();
            glScalef( textureScaleX, textureScaleY, 1.0f );

            glColor3f( 1.0f, 1.0f, 1.0f );
            glBegin( GL_QUADS );
                glTexCoord2f( 1.0f, 0.0f );
                glVertex2d( mAppWindowWidth, 0 );

                glTexCoord2f( 0.0f, 0.0f );
                glVertex2d( 0, 0 );

                glTexCoord2f( 0.0f, 1.0f );
                glVertex2d( 0, mAppWindowHeight );

                glTexCoord2f( 1.0f, 1.0f );
                glVertex2d( mAppWindowWidth, mAppWindowHeight );
            glEnd();

            glMatrixMode( GL_TEXTURE );
            glPopMatrix();

            glutSwapBuffers();
			glutPostRedisplay();
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        void mouseButton( int button, int state, int xIn, int yIn )
        {
            // texture is scaled to fit the screen so we scale mouse coords in the same way
			xIn = (xIn * mAppTextureWidth) / mAppWindowWidth;
			yIn = (yIn * mAppTextureHeight) / mAppWindowHeight;

			ChromiumDLL::MouseButtonType type = ChromiumDLL::MBT_LEFT;

            if ( button == GLUT_LEFT_BUTTON )
				type = ChromiumDLL::MBT_LEFT;
			else if (button == GLUT_RIGHT_BUTTON)
				type = ChromiumDLL::MBT_RIGHT;
			else if(button == GLUT_MIDDLE_BUTTON)
				type = ChromiumDLL::MBT_MIDDLE;

            if ( state == GLUT_DOWN )
            {
				pRenderer->onMouseClick(xIn, yIn, type, false, 1);
				pRenderer->onFocus(true);
            }
            else if ( state == GLUT_UP )
            {
				pRenderer->onMouseClick(xIn, yIn, type, true, 1);
            }
  
            glutPostRedisplay();
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        void mouseMove( int xIn , int yIn )
        {
			m_nLastXIn = (xIn * mAppTextureWidth) / mAppWindowWidth;
			m_nLastYIn = (yIn * mAppTextureHeight) / mAppWindowHeight;

			pRenderer->onMouseMove(m_nLastXIn, m_nLastYIn, false);

            glutPostRedisplay();
        }

        ////////////////////////////////////////////////////////////////////////////////
        //
        void keyboard( unsigned char keyIn, bool isDown)
        {
            // ESC key exits
            if ( keyIn == 27 )
            {
                reset();

                exit( 0 );
            }

			if (keyIn == 'f' && isDown)
			{
				pRenderer->getBrowser()->loadUrl("http://youtube.com");
			}

			if (keyIn == 'w' && isDown)
			{
				pRenderer->getBrowser()->scroll(m_nLastXIn, m_nLastYIn, 0, 120);
			}

			if (keyIn == 's' && isDown)
			{
				pRenderer->getBrowser()->scroll(m_nLastXIn, m_nLastYIn, 0, -120);
			}

			if (keyIn == 'a' && isDown)
			{
				pRenderer->getBrowser()->scroll(m_nLastXIn, m_nLastYIn, 120, 0);
			}

			if (keyIn == 'd' && isDown)
			{
				pRenderer->getBrowser()->scroll(m_nLastXIn, m_nLastYIn, -120, 0);
			}

			if (keyIn == 'i' && isDown)
			{
				pRenderer->getBrowser()->showInspector();
			}

			std::cout << "Key press was " << keyIn << " is down: " << isDown << std::endl;

			//if (isDown)
			//	pRenderer->onKeyPress(ChromiumDLL::KT_KEYDOWN, keyIn, 0, 0, 0);
			//else
			//	pRenderer->onKeyPress(ChromiumDLL::KT_KEYUP, keyIn, 0, 0, 0);
        }

		bool onNavigateUrl(const char* url, bool isMain) override
		{
			std::cout << "onNavigateUrl: " << url << std::endl;
			return true;
		}

		bool onNewWindowUrl(const char* url) override
		{
			std::cout << "onNewWindowUrl: " << url << std::endl;
			return true;
		}

		void onPageLoadStart() override
		{
			std::cout << "Page loading started" << std::endl;
		}
		void onPageLoadEnd() override
		{
			std::cout << "Page loading end" << std::endl;
		}

		bool onJScriptAlert(const char* msg) override
		{
			return true;
		}

		bool onJScriptConfirm(const char* msg, bool* result) override
		{
			return true;
		}

		bool onJScriptPrompt(const char* msg, const char* defualtVal, bool* handled, char result[255]) override
		{
			return true;
		}

		bool onKeyEvent(ChromiumDLL::KeyEventType type, int code, int modifiers, bool isSystemKey) override
		{
			return true;
		}

		void onLogConsoleMsg(const char* message, const char* source, int line) override
		{
			std::cout << "onLogConsoleMsg: " << message << std::endl;
		}

		void launchLink(const char* url) override
		{
			std::cout << "Link launched to: " << url << std::endl;
		}

		bool onLoadError(const char* errorMsg, const char* url, char* buff, size_t size) override
		{
			return true;
		}

		static void onMenuSelect(int nVal)
		{

		}

		inline bool HasAnyFlags(int value, int flags)
		{
			return (value&flags ? true : false);
		}

		bool handlePopupMenu(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumMenuInfoI>& menuInfo) override
		{
			std::cout << "HandlePopupMenu: " << m_nLastXIn << " " << m_nLastYIn << std::endl;

			//Have a look at desura: gcMenu* EventHandler::createMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo)

#ifdef WIN32
			HMENU hPopupMenu = CreatePopupMenu();
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, "Forward");
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 2, "Back");

			DestroyMenu(m_hMenu);
			m_hMenu = hPopupMenu;
#endif

			return true;
		}

		void onDownloadFile(const char* szUrl, const char* szMimeType, unsigned long long ullFileSize) override
		{
			std::cout << "Download file requested: " << szUrl << std::endl;
		}

		bool getAuthCredentials(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumAuthCredentialsI>& info)
		{
			std::cout << "getAuthCredentials: " << info->getPort() << " " << info->getHost() << " " << info->getScheme() << " " << info->getRealm() << " " << info->isProxy() << " " << std::endl;
			return false;
		}

		bool onProtocolExecution(const char* szUrl)
		{
			std::cout << "onProtocolExecution: " << szUrl << std::endl;
			return false;
		}


		void onStatus(const char* szStatus, ChromiumDLL::StatusType eType) override
		{
			//std::cout << "Status text changed to " << szStatus << " (type: " << eType << ")" << std::endl;
		}

		void onTitle(const char* szTitle) override
		{
			std::cout << "Title text changed to " << szTitle << std::endl;
		}

		bool onToolTip(const char* szToolTop) override
		{
			//std::cout << "Tool tip triggered: " << szToolTop << std::endl;
			return true;
		}

		bool getViewRect(int &x, int &y, int &w, int &h) override
		{
			std::cout << "getViewRect" << std::endl;

			x = glutGet(GLUT_WINDOW_X);
			y = glutGet(GLUT_WINDOW_Y);
			w = glutGet(GLUT_WINDOW_WIDTH);
			h = glutGet(GLUT_WINDOW_HEIGHT);

			return true;
		}

		void copyRegion(int x, int y, int w, int h, unsigned char* buffer)
		{
			for (int i = y; i < (y + h); ++i)
			{
				unsigned char* b = buffer + ((i * mAppTextureWidth) + x) * 4;
				unsigned char* p = mAppTexturePixels + ((i * mAppTextureWidth) + x) * 4;

				memcpy(p, b, w * 4);
			}
		}

		void onPaint(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumPaintInfo>& info) override
		{
			std::cout << "onPaint at " << info->windowRegion.x << " x " << info->windowRegion.y << " of size " << info->windowRegion.w << " x " << info->windowRegion.h << std::endl;

			{
				LockGaurd guard(m_BufferLock);

				mNeedsResize = mNeedsResize || mAppWindowHeight != info->windowRegion.h || mAppTextureWidth != info->windowRegion.w;
	
				mAppTextureWidth = info->windowRegion.w;
				mAppTextureHeight = info->windowRegion.h;

				if (mNeedsResize || info->regionCount == 0)
				{
					size_t totSize = mAppTextureWidth * mAppTextureHeight * 4;
					mAppTexturePixels = (unsigned char*)realloc(mAppTexturePixels, totSize);
					memcpy(mAppTexturePixels, info->buffer, totSize);
				}
				else
				{
					for (int x = 0; x < info->regionCount; ++x)
						copyRegion(info->invalidatedRegions[x].x, info->invalidatedRegions[x].y, info->invalidatedRegions[x].w, info->invalidatedRegions[x].h, (unsigned char*)info->buffer);
				}

				mNeedsUpdate = true;
			}

#ifdef SAVE_TO_BMP
			FILE* fh = fopen("out.bmp", "w");

			fprintf(fh, "BM");

			int size = w * h * 4 + 14;
			fwrite(&size, 4, 1, fh);

			fprintf(fh, "    ");

			int offset = 14 + sizeof(BITMAPCOREHEADER);
			fwrite(&offset, 4, 1, fh);


			BITMAPCOREHEADER bcm;
			bcm.bcSize = sizeof(BITMAPCOREHEADER);
			bcm.bcWidth = w;
			bcm.bcHeight = h;
			bcm.bcPlanes = 1;
			bcm.bcBitCount = 32;

			fwrite(&bcm, sizeof(BITMAPCOREHEADER), 1, fh);
			fwrite(buffer, w*h*4, 1, fh);

			fclose(fh);
#endif		
		}

		void onCursorChange(ChromiumDLL::eCursor cursor) override
		{
			if (cursor == ChromiumDLL::CURSOR_HAND)
				glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			else
				glutSetCursor(GLUT_CURSOR_INHERIT);
		}

		void onPUPaint(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumPaintInfo>& info) override
		{
			std::cout << "onPUPaint at " << info->windowRegion.x << " x " << info->windowRegion.y << " of size " << info->windowRegion.w << " x " << info->windowRegion.h << std::endl;
		}

		void onShow() override
		{
			std::cout << "onShow" << std::endl;
		}

		void onHide() override
		{
			std::cout << "onHide" << std::endl;
		}

		void onResize(int x, int y, int w, int h) override
		{
			std::cout << "onResize at " << x << " x " << y << " of size " << w << " x " << h << std::endl;
		}

        ////////////////////////////////////////////////////////////////////////////////
        //
        std::string getAppWindowName()
        {
            return mAppWindowName;
        }

    private:
        int mAppWindowWidth;
        int mAppWindowHeight;

        int mAppTextureWidth;
        int mAppTextureHeight;

		unsigned char* mAppTexturePixels;

        GLuint mAppTexture;
        std::string mAppWindowName;
        bool mNeedsUpdate;
		bool mNeedsResize;
		SharedObjectLoader g_CEFDll;
		typedef ChromiumDLL::ChromiumControllerI* (*CEF_InitFn)(bool, const char*, const char*, const char*);
		CEF_InitFn CEF_Init;
		ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererI> pRenderer;
		ChromiumDLL::ChromiumControllerI* pController;

		int m_nLastXIn;
		int m_nLastYIn;

		Mutex m_BufferLock;

#ifdef WIN32
		HWND m_hWnd;
		HMENU m_hMenu;
#endif
};

#ifdef WIN32
ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererI> cefGL::g_pRenderer;
#endif

ChromiumDLL::RefPtr<cefGL> theApp;

void glutReshape( int widthIn, int heightIn ) { theApp->reshape( widthIn, heightIn ); };
void glutDisplay() { if ( theApp ) theApp->display(); };
void glutKeyboard( unsigned char keyIn, int /*xIn*/, int /*yIn*/ ) { theApp->keyboard( keyIn, true ); };
void glutKeyboardUp( unsigned char keyIn, int /*xIn*/, int /*yIn*/ ) { theApp->keyboard( keyIn, false ); };
void glutMouseMove( int xIn , int yIn ) { theApp->mouseMove( xIn, yIn ); }
void glutMouseButton( int buttonIn, int stateIn, int xIn, int yIn ) { theApp->mouseButton( buttonIn, stateIn, xIn, yIn ); }
void glutIdle(){ theApp->idle(); }


int main( int argc, char* argv[] )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );

    theApp = new cefGL();

    glutInitWindowPosition( 80, 0 );
    glutInitWindowSize( 1024, 1024 );

    glutCreateWindow( theApp->getAppWindowName().c_str() );

    theApp->init();

    glutKeyboardFunc( glutKeyboard );
    glutKeyboardUpFunc( glutKeyboardUp );

    glutMouseFunc( glutMouseButton );
    glutPassiveMotionFunc( glutMouseMove );
    glutMotionFunc( glutMouseMove );

    glutDisplayFunc( glutDisplay );
    glutReshapeFunc( glutReshape );

    glutIdleFunc( glutIdle );
    glutMainLoop();

    return 0;
}
