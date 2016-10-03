// ChiChe.h

// Yes, I realize that this header is a terrible idea.
// I'm too lazy to dismantle it at the moment.

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
#include <wx/progdlg.h>
#include <wx/aboutdlg.h>
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>
#include <wx/time.h>
#include <wx/event.h>
#include <wx/dir.h>
#include <wx/filename.h>

// OpenGL:
#include <GL/gl.h>
#include <GL/glu.h>

// SDL:
#ifdef WIN32
#	include <SDL.h>
#else
#	include <SDL2/SDL.h>
#endif //WIN32

// STL:
#include <list>
#include <map>

// C3GA:
#include "c3ga/c3ga.h"

// Math:
#include <math.h>
#include <limits.h>

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
	class Sphere;
	class Sound;
	class Brain;
}

// ChiChe:
#include "ChiCheApp.h"
#include "ChiCheFrame.h"
#include "ChiCheSocket.h"
#include "ChiCheBoard.h"
#include "ChiCheClient.h"
#include "ChiCheServer.h"
#include "ChiCheCanvas.h"
#include "ChiCheSphere.h"
#include "ChiCheSound.h"
#include "ChiCheBrain.h"

// ChiChe.h