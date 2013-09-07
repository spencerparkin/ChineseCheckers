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
Client* App::GetClient( void )
{
	return client;
}

//=====================================================================================
Server* App::GetServer( void )
{
	return server;
}

// ChiCheApp.cpp