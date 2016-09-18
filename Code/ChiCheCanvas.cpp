// ChiCheCanvas.cpp

#include "ChiChe.h"

using namespace ChiChe;

int Canvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };

//=====================================================================================
Canvas::Canvas( wxWindow* parent ) : wxGLCanvas( parent, wxID_ANY, attributeList )
{
	context = 0;

	mousePos.x = 0;
	mousePos.y = 0;

	hitBuffer = 0;
	hitBufferSize = 0;

	lastFrameTime = -1;
	frameRate = 60.0;

	Bind( wxEVT_PAINT, &Canvas::OnPaint, this );
	Bind( wxEVT_SIZE, &Canvas::OnSize, this );
	Bind( wxEVT_MOUSEWHEEL, &Canvas::OnMouseWheel, this );
	Bind( wxEVT_LEFT_DOWN, &Canvas::OnMouseLeftDown, this );
	Bind( wxEVT_RIGHT_DOWN, &Canvas::OnMouseRightDown, this );
	Bind( wxEVT_MOTION, &Canvas::OnMouseMotion, this );
}

//=====================================================================================
/*virtual*/ Canvas::~Canvas( void )
{
	delete context;
}

//=====================================================================================
void Canvas::BindContext( bool actuallyBind )
{
	if( !context )
		context = new wxGLContext( this );

	if( context && actuallyBind )
		SetCurrent( *context );
}

//=====================================================================================
double Canvas::FrameRate( void )
{
	return frameRate;
}

//=====================================================================================
void Canvas::OnPaint( wxPaintEvent& event )
{
	PreRender( GL_RENDER );

	Client* client = wxGetApp().GetClient();
	if( client )
	{
		// Approximate the frame-rate this frame as the frame-rate last frame.
		wxLongLong thisFrameTime = wxGetLocalTimeMillis();
		if( lastFrameTime != -1 )
		{
			wxLongLong deltaTime = thisFrameTime - lastFrameTime;
			double frameTime = deltaTime.ToDouble() / 1000.0;
			frameRate = 1.0 / frameTime;
		}
		lastFrameTime = thisFrameTime;

		// Ask the client to render for us.
		client->Render( GL_RENDER );
	}

	PostRender( GL_RENDER );
}

//=====================================================================================
void Canvas::PreRender( GLenum renderMode )
{
	BindContext( true );

	glClearColor( 0.f, 0.f, 0.f, 1.f );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glHint( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );
	glShadeModel( GL_SMOOTH );

	if( renderMode == GL_SELECT )
	{
		hitBufferSize = 512;
		hitBuffer = new unsigned int[ hitBufferSize ];
		glSelectBuffer( hitBufferSize, hitBuffer );

		glRenderMode( GL_SELECT );
		glInitNames();
	}

	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	wxPoint* pickingPoint = 0;
	if( renderMode == GL_SELECT )
		pickingPoint = &mousePos;
	camera.LoadViewingMatrices( pickingPoint );
}

//=====================================================================================
void Canvas::PostRender( GLenum renderMode )
{
	glFlush();

	if( renderMode == GL_SELECT )
	{
		int hitCount = glRenderMode( GL_RENDER );

		Client* client = wxGetApp().GetClient();
		if( client )
			client->ProcessHitList( hitBuffer, hitBufferSize, hitCount );

		delete[] hitBuffer;
		hitBuffer = 0;
		hitBufferSize = 0;
	}
	else if( renderMode == GL_RENDER )
	{
		SwapBuffers();
	}
}

//=====================================================================================
void Canvas::OnSize( wxSizeEvent& event )
{
#ifdef __LINUX__
	// TODO: Why can't or shouldn't we bind the context in the on-size event for Linux?  OpenGL must be bound before issuing gl-commands.
	BindContext( false );
#else //__LINUX__
	BindContext( true );
#endif //__LINUX__
	wxSize size = event.GetSize();
	glViewport( 0, 0, size.GetWidth(), size.GetHeight() );
	double aspectRatio = double( size.GetWidth() ) / double( size.GetHeight() );
	camera.SetAspectRatio( aspectRatio );
	Refresh();
}

//=====================================================================================
void Canvas::OnMouseWheel( wxMouseEvent& event )
{
	double wheelDelta = event.GetWheelDelta();
	double wheelRotation = event.GetWheelRotation();
	double wheelIncrements = wheelRotation / wheelDelta;
	double zoomPercentagePerIncrement = 0.1;
	double zoomPercentage = 1.0 + wheelIncrements * zoomPercentagePerIncrement;
	double minimumFocalLength = 5.0;
	double maximumFocalLength = 100.0;
	camera.Zoom( zoomPercentage, minimumFocalLength, maximumFocalLength );
	Refresh();
}

//=====================================================================================
void Canvas::OnMouseLeftDown( wxMouseEvent& event )
{
	SetFocus();

	mousePos = event.GetPosition();
}

//=====================================================================================
void Canvas::OnMouseRightDown( wxMouseEvent& event )
{
	mousePos = event.GetPosition();

	Client* client = wxGetApp().GetClient();
	if( client )
	{
		PreRender( GL_SELECT );
		client->Render( GL_SELECT );
		PostRender( GL_SELECT );
	}
}

//=====================================================================================
void Canvas::OnMouseMotion( wxMouseEvent& event )
{
	if( event.LeftIsDown() )
	{
		wxPoint mouseDelta = event.GetPosition() - mousePos;
		mousePos = event.GetPosition();

		c3ga::vectorE3GA eye = camera.GetEye();

		c3ga::vectorE3GA xAxis, yAxis, zAxis;
		camera.CalcPanFrame( xAxis, yAxis, zAxis );

		double unitsPerPixel = 0.2;

		c3ga::vectorE3GA delta;
		delta.set( c3ga::vectorE3GA::coord_e1_e2_e3, unitsPerPixel * -double( mouseDelta.x ), unitsPerPixel * double( mouseDelta.y ), 0.0 );
		delta = c3ga::gp( xAxis, delta.get_e1() ) + c3ga::gp( yAxis, delta.get_e2() ) + c3ga::gp( zAxis, delta.get_e3() );
		camera.Move( delta, true );

		c3ga::vectorE3GA lookVec = c3ga::unit( camera.GetFocus() - camera.GetEye() );
		double angle = acos( lookVec.get_e2() );
		if( angle < M_PI / 2.0 || angle > M_PI * 0.95 )
			camera.SetEye( eye );
		else
			Refresh();
	}
}

//=====================================================================================
Canvas::Camera::Camera( void )
{
	focus.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 0.0, 0.0 );
	eye.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 20.0, -50.0 );
	up.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 1.0, 0.0 );
	
	foviAngle = 60.0;
	aspectRatio = 1.0;
}

//=====================================================================================
Canvas::Camera::~Camera( void )
{
}

//=====================================================================================
void Canvas::Camera::Zoom( double zoomPercentage, double minimumFocalLength, double maximumFocalLength )
{
	c3ga::vectorE3GA originalEye = eye;
	c3ga::vectorE3GA lookVec = focus - eye;
	lookVec = c3ga::gp( lookVec, zoomPercentage );
	eye = focus - lookVec;
	double focalLength = FocalLength();
	if( focalLength < minimumFocalLength || focalLength > maximumFocalLength )
		eye = originalEye;
}

//=====================================================================================
double Canvas::Camera::FocalLength( void )
{
	c3ga::vectorE3GA lookVec = focus - eye;
	return c3ga::norm( lookVec );
}

//=====================================================================================
void Canvas::Camera::Move( const c3ga::vectorE3GA& delta, bool maintainFocalLength /*= true*/ )
{
	double oldFocalLength = FocalLength();
	eye = eye + delta;
	if( maintainFocalLength )
	{
		double newFocalLength = FocalLength();
		double zoomPercentage = oldFocalLength / newFocalLength;
		Zoom( zoomPercentage, 0.0, 1000.0 );
	}
}

//=====================================================================================
void Canvas::Camera::CalcPanFrame( c3ga::vectorE3GA& xAxis, c3ga::vectorE3GA& yAxis, c3ga::vectorE3GA& zAxis )
{
	// This should create a right-handed system.
	c3ga::vectorE3GA lookVec = focus - eye;
	zAxis = c3ga::unit( c3ga::negate( lookVec ) );
	yAxis = c3ga::unit( c3ga::lc( lookVec, c3ga::op( lookVec, up ) ) );
	xAxis = c3ga::gp( c3ga::op( zAxis, yAxis ), c3ga::I3 );
}

//=====================================================================================
void Canvas::Camera::LoadViewingMatrices( wxPoint* pickingPoint /*= 0*/ )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	if( pickingPoint )
	{
		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		GLdouble x = pickingPoint->x;
		GLdouble y = GLdouble( viewport[3] ) - GLdouble( pickingPoint->y );
		GLdouble w = 2.0;
		GLdouble h = 2.0;
		gluPickMatrix( x, y, w, h, viewport );
	}
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

//=====================================================================================
const c3ga::vectorE3GA& Canvas::Camera::GetEye( void )
{
	return eye;
}

//=====================================================================================
void Canvas::Camera::SetEye( const c3ga::vectorE3GA& eye )
{
	this->eye = eye;
}

//=====================================================================================
const c3ga::vectorE3GA& Canvas::Camera::GetFocus( void )
{
	return focus;
}

//=====================================================================================
void Canvas::Camera::SetFocus( const c3ga::vectorE3GA& focus )
{
	this->focus = focus;
}

// ChiCheCanvas.cpp
