// ChiCheBoard.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
int Board::layoutMap[ LAYOUT_SIZE ][ LAYOUT_SIZE ] =
{
	// These entries correspond to the enum in the class definition!
	{ -1, -1, -1, -1,  1, -1, -1, -1, -1  -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  1,  1, -1, -1, -1  -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  1,  1,  1, -1, -1  -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{  2,  2,  2,  2,  0,  0,  0,  0,  0,  6,  6,  6,  6, -1, -1, -1, -1 },
	{ -1,  2,  2,  2,  0,  0,  0,  0,  0,  0,  6,  6,  6, -1, -1, -1, -1 },
	{ -1, -1,  2,  2,  0,  0,  0,  0,  0,  0,  0,  6,  6, -1, -1, -1, -1 },
	{ -1, -1, -1,  2,  0,  0,  0,  0,  0,  0,  0,  0,  6, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  3,  0,  0,  0,  0,  0,  0,  0,  0,  5, -1, -1, -1 },
	{ -1, -1, -1, -1,  3,  3,  0,  0,  0,  0,  0,  0,  0,  5,  5, -1, -1 }, 
	{ -1, -1, -1, -1,  3,  3,  3,  0,  0,  0,  0,  0,  0,  5,  5,  5, -1 },
	{ -1, -1, -1, -1,  3,  3,  3,  3,  0,  0,  0,  0,  0,  5,  5,  5,  5 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,  4,  4,  4,  4, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  4,  4,  4, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  4,  4, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1 },
};

//=====================================================================================
Board::Board( int participants, bool animate )
{
	this->participants = participants;

	whosTurn = NONE;
	NextTurn();

	GenerateGraph( animate );
}

//=====================================================================================
Board::~Board( void )
{
}

//=====================================================================================
void Board::NextTurn( void )
{
	IncrementTurn();
	while( !IsParticipant( whosTurn ) )
		IncrementTurn();
}

//=====================================================================================
void Board::IncrementTurn( void )
{
	whosTurn++;
	if( whosTurn > CYAN )
		whosTurn = RED;
}

//=====================================================================================
bool Board::IsParticipant( int color )
{
	if( participants & ( 1 << color ) )
		return true;
	return false;
}

//=====================================================================================
/*static*/ int Board::ZoneTarget( int color )
{
	// This is baed on the layout map!
	return ( color + 2 ) % 6 + 1;
}

//=====================================================================================
void Board::GenerateGraph( bool animate )
{
	int locationID = 0;
	Location* connectionMap[ LAYOUT_SIZE ][ LAYOUT_SIZE ];

	// Create all the locations.
	for( int i = 0; i < LAYOUT_SIZE; i++ )
	{
		for( int j = 0; j < LAYOUT_SIZE; j++ )
		{
			connectionMap[i][j] = 0;
			int color = layoutMap[i][j];
			if( color != -1 )
			{
				Location* location = new Location( color, color, locationID );
				locationMap[ locationID ] = location;
				connectionMap[i][j] = location;
				locationID++;
			}
		}
	}

	// Connect all the locations.
	for( int i = 0; i < LAYOUT_SIZE; i++ )
	{
		for( int j = 0; j < LAYOUT_SIZE; j++ )
		{
			Location* location = connectionMap[i][j];
			if( location )
			{
				// The order here is significant.  We just need a CW or CCW order.
				if( j < LAYOUT_SIZE - 1 )
					location->SetAdjacency( 0, connectionMap[ i ][ j + 1 ] );
				if( i > 0 )
					location->SetAdjacency( 1, connectionMap[ i - 1 ][ j ] );
				if( i > 0 && j > 0 )
					location->SetAdjacency( 2, connectionMap[ i - 1 ][ j - 1 ] );
				if( j > 0 )
					location->SetAdjacency( 3, connectionMap[ i ][ j - 1 ] );
				if( i < LAYOUT_SIZE - 1 )
					location->SetAdjacency( 4, connectionMap[ i + 1 ][ j ] );
				if( i < LAYOUT_SIZE - 1 && j < LAYOUT_SIZE - 1 )
					location->SetAdjacency( 5, connectionMap[ i + 1 ][ j + 1 ] );
			}
		}
	}

	// Calculate the spacial positions of each location in the graph.
	Location* center = connectionMap[ ( LAYOUT_SIZE - 1 ) / 2 ][ ( LAYOUT_SIZE - 1 ) / 2 ];
	c3ga::vectorE3GA centerPosition( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 0.0, 0.0 );
	VisitationMap visitationMap;
	center->CalcPositionOfAdjacencies( centerPosition, visitationMap );

	// If the board will be rendered, create the pieces that animate.
	if( animate )
	{
		//...
	}
}

//=====================================================================================
// Here we return the first winner that we find.  The state of the board should
// never be allowed to reach a point where there are multiple winners.
int Board::DetermineWinner( void )
{
	typedef std::map< int, int > TargetCountMap;
	TargetCountMap targetCountMap;

	for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end(); iter++ )
	{
		Location* location = iter->second;
		if( location->GetOccupant() != NONE && location->GetZone() != NONE )
		{
			int occupant = location->GetOccupant();
			int zone = location->GetZone();
			if( ZoneTarget( occupant ) == zone )
			{
				TargetCountMap::iterator iter = targetCountMap.find( occupant );
				if( iter == targetCountMap.end() )
					targetCountMap[ occupant ] = 1;
				else
					iter->second++;
			}
		}
	}

	for( TargetCountMap::iterator iter = targetCountMap.begin(); iter != targetCountMap.end(); iter++ )
	{
		int count = iter->second;
		if( count == PIECES_PER_ZONE )
		{
			int color = iter->first;
			return color;
		}
	}

	return -1;
}

//=====================================================================================
void Board::Render( GLenum renderMode )
{
	for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end(); iter++ )
	{
		Location* location = iter->second;
		location->Render( renderMode );
	}
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
Board::Location::Location( int occupant, int zone, int locationID )
{
	this->occupant = occupant;
	this->zone = zone;
	this->locationID = locationID;
	position.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 0.0, 0.0 );
	piece = 0;

	for( int i = 0; i < ADJACENCIES; i++ )
		adjacency[i] = 0;
}

//=====================================================================================
Board::Location::~Location( void )
{
}

//=====================================================================================
void Board::Location::CalcPositionOfAdjacencies( const c3ga::vectorE3GA& position, VisitationMap& visitationMap )
{
	this->position = position;
	visitationMap[ locationID ] = true;

	for( int i = 0; i < ADJACENCIES; i++ )
	{
		Location* adjLocation = adjacency[i];
		if( !adjLocation )
			continue;
		
		if( visitationMap.end() == visitationMap.find( adjLocation->locationID ) )
			continue;

		double angle = double(i) / double( ADJACENCIES ) * 2.0 * 3.1415926536;
		double radius = double( LOCATION_EDGE_LENGTH );
		c3ga::vectorE3GA adjPosition;
		adjPosition.set( c3ga::vectorE3GA::coord_e1_e2_e3, radius * cos( angle ), 0.0, radius * sin( angle ) );
		adjLocation->CalcPositionOfAdjacencies( adjPosition, visitationMap );
	}
}

//=====================================================================================
void Board::Location::SetAdjacency( int index, Location* location )
{
	if( 0 <= index && index < ADJACENCIES )
		adjacency[ index ] = location;
}

//=====================================================================================
Board::Location* Board::Location::GetAdjacency( int index )
{
	if( 0 <= index && index < ADJACENCIES )
		return adjacency[ index ];
	return 0;
}

//=====================================================================================
int Board::Location::GetZone( void )
{
	return zone;
}

//=====================================================================================
void Board::Location::SetOccupant( int occupant )
{
	this->occupant = occupant;
}

//=====================================================================================
int Board::Location::GetOccupant( void )
{
	return occupant;
}

//=====================================================================================
void Board::Location::Render( GLenum renderMode )
{
	if( renderMode == GL_RENDER )
	{
		switch( zone )
		{
			case NONE:		glColor3f( 1.f, 1.f, 1.f );		break;
			case RED:		glColor3f( 1.f, 0.f, 0.f );		break;
			case GREEN:		glColor3f( 0.f, 1.f, 0.f );		break;
			case BLUE:		glColor3f( 0.f, 0.f, 1.f );		break;
			case YELLOW:	glColor3f( 1.f, 1.f, 0.f );		break;
			case MAGENTA:	glColor3f( 1.f, 0.f, 1.f );		break;
			case CYAN:		glColor3f( 0.f, 1.f, 1.f );		break;
		}

		glBegin( GL_LINES );

		for( int i = 0; i < ADJACENCIES; i++ )
		{
			Location* adjLocation = adjacency[i];
			if( adjLocation )
			{
				c3ga::vectorE3GA midPoint = c3ga::add( c3ga::gp( position, 0.5 ), c3ga::gp( adjLocation->position, 0.5 ) );
				glVertex3f( position.get_e1(), position.get_e2(), position.get_e3() );
				glVertex3f( midPoint.get_e1(), midPoint.get_e2(), midPoint.get_e3() );
			}
		}

		glEnd();
	}
	else if( renderMode == GL_SELECT )
	{
		// In this case, we want to render a hot-spot hexagon.
		
	}
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