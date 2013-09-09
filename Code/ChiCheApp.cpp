// ChiCheApp.cpp

#include "ChiChe.h"

namespace ChiChe
{
	IMPLEMENT_APP( App )
}

using namespace ChiChe;

//=====================================================================================
App::App( void )
{
	frame = 0;
	client = 0;
	server = 0;
}

//=====================================================================================
App::~App( void )
{
}

//=====================================================================================
/*virtual*/ bool App::OnInit( void )
{
	if( !wxApp::OnInit() )
		return false;

	frame = new Frame();
	frame->Show();

	return true;
}

//=====================================================================================
void App::SetClient( Client* client )
{
	this->client = client;
}

//=====================================================================================
Client* App::GetClient( void )
{
	return client;
}

//=====================================================================================
void App::SetServer( Server* server )
{
	this->server = server;

	wxStatusBar* statusBar = frame->GetStatusBar();
	if( server )
		statusBar->SetStatusText( wxT( "Game server online and running!" ) );
	else
		statusBar->SetStatusText( wxT( "Game server offline!" ) );
}

//=====================================================================================
Server* App::GetServer( void )
{
	return server;
}

//=====================================================================================
Frame* App::GetFrame( void )
{
	return frame;
}

//=====================================================================================
/*virtual*/ int App::OnExit( void )
{
	return wxApp::OnExit();
}

// ChiCheApp.cpp