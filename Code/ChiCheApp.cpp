// ChiCheApp.cpp

#include "ChiChe.h"

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

IMPLEMENT_APP( ChiChe::App )

// ChiCheApp.cpp
