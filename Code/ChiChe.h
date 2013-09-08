// ChiChe.h

// wxWidgets:
#include <wx/setup.h>
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/socket.h>
#include <wx/event.h>
#include <wx/menu.h>
#include <wx/timer.h>
#include <wx/sizer.h>
#include <wx/glcanvas.h>
#include <wx/choicdlg.h>
#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/tokenzr.h>
#include <wx/progdlg.h>

// OpenGL:
#include <GL/gl.h>
#include <GL/glu.h>

// STL:
#include <list>
#include <map>

// C3GA:
#include "c3ga/c3ga.h"

// Forwards:
namespace ChiChe
{
	class App;
	class Frame;
	class Socket;
	class Board;
	class Client;
	class Server;
	class Canvas;
}

// ChiChe:
#include "ChiCheApp.h"
#include "ChiCheFrame.h"
#include "ChiCheSocket.h"
#include "ChiCheBoard.h"
#include "ChiCheClient.h"
#include "ChiCheServer.h"
#include "ChiCheCanvas.h"

// ChiChe.h