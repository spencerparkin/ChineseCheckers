// ChiCheBoard.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Board::Board( int participants )
{
}

//=====================================================================================
Board::~Board( void )
{
}

//=====================================================================================
int Board::DetermineWinner( void )
{
	return -1;
}

//=====================================================================================
void Board::Render( GLenum renderMode )
{
}

//=====================================================================================
void Board::Animate( double frameRate )
{
}

//=====================================================================================
int Board::FindSelectedLocation( unsigned int* hitBuffer, int hitBufferSize, int hitCount )
{
	int locationID = -1;
	return locationID;
}

//=====================================================================================
bool Board::FindMoveSequence( int sourceID, int destinationID, MoveSequence& moveSequence )
{
	moveSequence.clear();

	return true;
}

//=====================================================================================
bool Board::IsMoveSequenceValid( const MoveSequence& moveSequence )
{
	return true;
}

//=====================================================================================
bool Board::ApplyMoveSequence( const MoveSequence& moveSequence )
{
	return true;
}

//=====================================================================================
Board::Location::Location( const c3ga::vectorE3GA& position, int occupant, int zone, int locationID )
{
}

//=====================================================================================
Board::Location::~Location( void )
{
}

//=====================================================================================
void Board::Location::Render( GLenum renderMode )
{
}

//=====================================================================================
Board::Piece::Piece( int color )
{
}

//=====================================================================================
Board::Piece::~Piece( void )
{
}

//=====================================================================================
void Board::Piece::Render( void )
{
}

//=====================================================================================
void Board::Piece::Animate( double frameRate )
{
}

// ChiCheBoard.cpp