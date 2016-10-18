// ChiCheApp.cpp

#include "ChiCheApp.h"
#include "ChiCheFrame.h"
#include "ChiCheSound.h"
#include "ChiCheMongo.h"
#include <wx/setup.h>

using namespace ChiChe;

//=====================================================================================
App::App( void )
{
	frame = 0;
	client = 0;
	server = 0;
	sound = nullptr;
	soundEffect = "Doink";
}

//=====================================================================================
App::~App( void )
{
	if( sound )
		sound->Shutdown();
	delete sound;

	Mongo::CleanUp();
}

//=====================================================================================
/*virtual*/ bool App::OnInit( void )
{
	srand( ( unsigned )time( nullptr ) );

	if( !wxApp::OnInit() )
		return false;

	if( !Mongo::Init() )
	{
		//return false;
	}
	
	sound = new Sound();
	if( !sound->Setup() )
	{
		//return false;
	}

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
Sound* App::GetSound( void )
{
	return sound;
}

//=====================================================================================
/*virtual*/ int App::OnExit( void )
{
	return wxApp::OnExit();
}

//=====================================================================================
IMPLEMENT_APP( ChiChe::App )

// ChiCheApp.cpp
