// ChiCheCanvas.cpp

#include "ChiChe.h"

using namespace ChiChe;

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

//=====================================================================================
Canvas::Canvas( wxWindow* parent ) : wxGLCanvas( parent, wxID_ANY, attributeList )
{
	context = 0;

	Bind( wxEVT_PAINT, &Canvas::OnPaint, this );
	Bind( wxEVT_SIZE, &Canvas::OnSize, this );
}

//=====================================================================================
/*virtual*/ Canvas::~Canvas( void )
{
	delete context;
}

//=====================================================================================
void Canvas::BindContext( void )
{
	if( !context )
		context = new wxGLContext( this );

	SetCurrent( *context );
}

//=====================================================================================
void Canvas::OnPaint( wxPaintEvent& event )
{
	PreRender( GL_RENDER );

	Client* client = wxGetApp().GetClient();
	if( client )
		client->Render( GL_RENDER );
	else
		RenderAxes();

	PostRender( GL_RENDER );
}

//=====================================================================================
void Canvas::RenderAxes( void )
{
	glBegin( GL_LINES );

	glColor3f( 1.f, 0.f, 0.f );
	glVertex3f( -10.f, 0.f, 0.f );
	glVertex3f( 10.f, 0.f, 0.f );
	
	glColor3f( 0.f, 1.f, 0.f );
	glVertex3f( 0.f, -10.f, 0.f );
	glVertex3f( 0.f, 10.f, 0.f );

	glColor3f( 0.f, 0.f, 1.f );
	glVertex3f( 0.f, 0.f, -10.f );
	glVertex3f( 0.f, 0.f, 10.f );

	glEnd();
}

//=====================================================================================
void Canvas::PreRender( GLenum renderMode )
{
	BindContext();

	glClearColor( 0.f, 0.f, 0.f, 1.f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	camera.LoadViewingMatrices();
}

//=====================================================================================
void Canvas::PostRender( GLenum renderMode )
{
	glFlush();
	SwapBuffers();
}

//=====================================================================================
void Canvas::OnSize( wxSizeEvent& event )
{
	BindContext();
	wxSize size = event.GetSize();
	glViewport( 0, 0, size.GetWidth(), size.GetHeight() );
	double aspectRatio = double( size.GetWidth() ) / double( size.GetHeight() );
	camera.SetAspectRatio( aspectRatio );
	Refresh();
}

//=====================================================================================
Canvas::Camera::Camera( void )
{
	focus.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 0.0, 0.0 );
	eye.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 0.0, -20.0 );
	up.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 1.0, 0.0 );
	
	foviAngle = 60.0;
	aspectRatio = 1.0;
}

//=====================================================================================
Canvas::Camera::~Camera( void )
{
}

//=====================================================================================
void Canvas::Camera::LoadViewingMatrices( void )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( foviAngle, aspectRatio, 1.0, 1000.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt( eye.get_e1(), eye.get_e2(), eye.get_e3(),
				focus.get_e1(), focus.get_e2(), focus.get_e3(),
				up.get_e1(), up.get_e2(), up.get_e3() );
}

//=====================================================================================
void Canvas::Camera::SetAspectRatio( double aspectRatio )
{
	this->aspectRatio = aspectRatio;
}

//=====================================================================================
double Canvas::Camera::GetAspectRatio( void )
{
	return aspectRatio;
}

// ChiCheCanvas.cpp