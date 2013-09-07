// ChiCheClient.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Client::Client( Type type )
{
	this->type = type;
	socket = 0;
	board = 0;
	color = Board::NONE;
}

//=====================================================================================
Client::~Client( void )
{
}

//=====================================================================================
bool Client::Connect( const wxIPV4address& address )
{
	return true;
}

//=====================================================================================
bool Client::Run( void )
{
	return true;
}

//=====================================================================================
bool Client::Render( GLenum renderMode )
{
	return true;
}

// ChiCheClient.cpp