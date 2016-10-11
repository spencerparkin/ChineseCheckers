// ChiCheBrain.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Brain::Brain( void )
{
}

//=====================================================================================
/*virtual*/ Brain::~Brain( void )
{
}

//=====================================================================================
bool Brain::FindGoodMoveForParticipant( int color, Board* board, Board::Move& move )
{
	return false;
}

// ChiCheBrain.cpp