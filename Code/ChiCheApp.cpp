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

	return true;
}

// ChiCheApp.cpp