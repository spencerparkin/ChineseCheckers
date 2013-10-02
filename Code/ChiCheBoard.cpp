// ChiCheBoard.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
int Board::layoutMap[ LAYOUT_SIZE ][ LAYOUT_SIZE ] =
{
	// These entries correspond to the enum in the class definition!
	{ -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
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
	this->animate = animate;

	whosTurn = NONE;
	NextTurn();

	CreateGraph();
}

//=====================================================================================
Board::~Board( void )
{
	DestroyGraph();
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
bool Board::IsParticipantsTurn( int color )
{
	if( color == whosTurn )
		return true;
	return false;
}

//=====================================================================================
int Board::GetParticipants( void )
{
	return participants;
}

//=====================================================================================
int Board::WhosTurn( void )
{
	return whosTurn;
}

//=====================================================================================
/*static*/ int Board::ZoneTarget( int color )
{
	// This is baed on the layout map!
	return ( color + 2 ) % 6 + 1;
}

//=====================================================================================
/*static*/ void Board::ParticipantText( int color, wxString& textColor )
{
	textColor = wxT( "???" );
	switch( color )
	{
		case NONE:		textColor = wxT( "none" );		break;
		case RED:		textColor = wxT( "red" );		break;
		case GREEN:		textColor = wxT( "green" );		break;
		case BLUE:		textColor = wxT( "blue" );		break;
		case YELLOW:	textColor = wxT( "yellow" );	break;
		case MAGENTA:	textColor = wxT( "magenta" );	break;
		case CYAN:		textColor = wxT( "cyan" );		break;
	}
}

//=====================================================================================
/*static*/ bool Board::UnpackMove( const Socket::Packet& inPacket, wxInt32& sourceID, wxInt32& destinationID )
{
	if( inPacket.GetType() != Client::GAME_MOVE && inPacket.GetType() != Server::GAME_MOVE )
		return false;
		
	if( inPacket.GetSize() != 2 * sizeof( wxInt32 ) )
		return false;
	
	wxInt32* data = ( wxInt32* )inPacket.GetData();
	if( !data )
		return false;

	sourceID = data[0];
	destinationID = data[1];
	if( inPacket.ByteSwap() )
	{
		sourceID = wxINT32_SWAP_ALWAYS( sourceID );
		destinationID = wxINT32_SWAP_ALWAYS( destinationID );
	}

	return true;
}

//=====================================================================================
/*static*/ bool Board::PackMove( Socket::Packet& outPacket, wxInt32 sourceID, wxInt32 destinationID, int packetType )
{
	outPacket.Reset();

	if( packetType != Client::GAME_MOVE && packetType != Server::GAME_MOVE )
		return false;

	wxInt32* data = new wxInt32[2];
	data[0] = sourceID;
	data[1] = destinationID;

	outPacket.SetType( packetType );
	outPacket.SetData( ( wxInt8* )data );
	outPacket.SetSize( sizeof( wxInt32 ) * 2 );
	outPacket.OwnsMemory( true );

	return true;
}

//=====================================================================================
int Board::OccupantAtLocation( int locationID )
{
	LocationMap::iterator iter = locationMap.find( locationID );
	if( iter == locationMap.end() )
		return -1;

	Location* location = iter->second;
	return location->GetOccupant();
}

//=====================================================================================
bool Board::PositionAtLocation( int locationID, c3ga::vectorE3GA& position )
{
	LocationMap::iterator iter = locationMap.find( locationID );
	if( iter == locationMap.end() )
		return false;

	Location* location = iter->second;
	position = location->GetPosition();
	return true;
}

//=====================================================================================
void Board::CreateGraph( void )
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
				int occupant = NONE;
				if( IsParticipant( color ) )
					occupant = color;
				Location* location = new Location( occupant, color, locationID );
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

	// If the board will be animated/rendered, create the pieces that animate.
	if( animate )
		CreatePieces();
}

//=====================================================================================
void Board::DestroyGraph( void )
{
	// The pieces must be destroyed before the locations.
	if( animate )
		DestroyPieces();

	while( locationMap.size() > 0 )
	{
		LocationMap::iterator iter = locationMap.begin();
		Location* location = iter->second;
		delete location;
		locationMap.erase( iter );
	}
}

//=====================================================================================
void Board::CreatePieces( void )
{
	if( pieceList.size() == 0 )
	{
		for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end(); iter++ )
		{
			Location* location = iter->second;
			int occupant = location->GetOccupant();
			if( occupant != NONE && IsParticipant( occupant ) )
			{
				Piece* piece = new Piece( location->GetLocationID() );
				location->SetPiece( piece );
				pieceList.push_back( piece );
			}
		}
	}
}

//=====================================================================================
void Board::DestroyPieces( void )
{
	while( pieceList.size() > 0 )
	{
		PieceList::iterator iter = pieceList.begin();
		Piece* piece = *iter;
		int locationID = piece->GetLocationID();
		Location* location = locationMap[ locationID ];
		location->SetPiece(0);
		delete piece;
		pieceList.erase( iter );
	}
}

//=====================================================================================
bool Board::GetGameState( Socket::Packet& outPacket )
{
	outPacket.Reset();

	wxInt32 size = locationMap.size() * 2 + 1;
	wxInt32* locationDataArray = new wxInt32[ size ];
	int index = 0;
	for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end(); iter++ )
	{
		wxInt32* locationData = &locationDataArray[ index++ * 2 ];
		Location* location = iter->second;
		locationData[0] = iter->first;
		locationData[1] = location->GetOccupant();
	}

	locationDataArray[ size - 1 ] = whosTurn;

	outPacket.SetType( Server::GAME_STATE );
	outPacket.SetData( ( wxInt8* )locationDataArray );
	outPacket.SetSize( size * sizeof( wxInt32 ) );
	outPacket.OwnsMemory( true );

	return true;
}

//=====================================================================================
bool Board::SetGameState( const Socket::Packet& inPacket )
{
	if( inPacket.GetType() != Server::GAME_STATE )
		return false;

	wxInt32 size = locationMap.size() * 2 + 1;
	if( inPacket.GetSize() != size * sizeof( wxInt32 ) )
		return false;

	if( animate )
		DestroyPieces();

	const wxInt32* locationDataArray = ( const int* )inPacket.GetData();
	for( int index = 0; index < size - 1; index += 2 )
	{
		const wxInt32* locationData = &locationDataArray[ index ];
		wxInt32 locationID = locationData[0];
		wxInt32 occupant = locationData[1];
		if( inPacket.ByteSwap() )
		{
			locationID = wxINT32_SWAP_ALWAYS( locationID );
			occupant = wxINT32_SWAP_ALWAYS( occupant );
		}
		LocationMap::iterator iter = locationMap.find( locationID );
		if( iter == locationMap.end() )
			return false;		// We might be leaving the game in a corrupted state!

		Location* location = iter->second;
		location->SetOccupant( occupant );
	}

	if( animate )
		CreatePieces();

	whosTurn = locationDataArray[ size - 1 ];

	return true;
}

//=====================================================================================
Board::Location* Board::FindZoneVertex( int zone )
{
	Location* foundLocation = 0;
	for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end() && !foundLocation; iter++ )
	{
		Location* location = iter->second;
		if( location->GetZone() == zone && location->GetAdjacencyCount() == 2 )
			foundLocation = location;
	}
	return foundLocation;
}

//=====================================================================================
// This algorithm doesn't try to anticipate a player's actions, think ahead, or anything
// like that.  But one advantage it does have over a human player is that it can make
// a decision based upon an evaluation of every possible move.  There is certainly
// room for improvement here.
bool Board::FindBestMoveForParticipant( int color, int& sourceID, int& destinationID )
{
	// Calculate the general direction that we want to be moving.
	int zoneTarget = ZoneTarget( color );
	Location* sourceVertexLocation = FindZoneVertex( color );
	Location* destinationVertexLocation = FindZoneVertex( zoneTarget );
	if( !sourceVertexLocation || !destinationVertexLocation )
		return false;		// This should never happen.
	c3ga::vectorE3GA generalMoveDirection = c3ga::unit( destinationVertexLocation->GetPosition() - sourceVertexLocation->GetPosition() );

	// Our target location is then an unoccupied location as close to the destination vertex location as possible.
	Location* targetLocation = FindClosestUnoccupiedLocationInZone( destinationVertexLocation->GetPosition(), zoneTarget );

	// In this case, we have either already won the game, or one or more opponents are completely occupying our target zone.
	if( !targetLocation )
	{
		// Go ahead and choose a target location that is closest to our target vertex in the nuetral zone.
		targetLocation = FindClosestUnoccupiedLocationInZone( destinationVertexLocation->GetPosition(), NONE );
		if( !targetLocation )
			return false;
	}

	// Go create a big huge list of every possible move that we can possibly make.
	SourceMap sourceMap;
	FindAllMovesForParticipant( color, sourceMap );

	// Now try to choose the best move from among this list.
	// It is not clear as to the correctness of this algorithm, which would be that it always converges to a win.
	double largestMoveDistance = -1.0;
	int bestMove[2] = { -1, -1 };
	for( SourceMap::iterator sourceMapIter = sourceMap.begin(); sourceMapIter != sourceMap.end(); sourceMapIter++ )
	{
		sourceID = sourceMapIter->first;
		DestinationMap* destinationMap = sourceMapIter->second;
		for( DestinationMap::iterator destinationMapIter = destinationMap->begin(); destinationMapIter != destinationMap->end(); destinationMapIter++ )
		{
			destinationID = destinationMapIter->first;
			int edgeCount = destinationMapIter->second;

			Location* sourceLocation = locationMap[ sourceID ];
			Location* destinationLocation = locationMap[ destinationID ];

			c3ga::vectorE3GA moveDirection = destinationLocation->GetPosition() - sourceLocation->GetPosition();

			// Rule out a move right away if it is a digression of our objective.
			double epsilon = 0.5;
			double projectedMoveDistance = c3ga::lc( generalMoveDirection, c3ga::unit( moveDirection ) );
			if( projectedMoveDistance < -epsilon )
				continue;
			if( !( destinationLocation->GetZone() == NONE || destinationLocation->GetZone() == zoneTarget || destinationLocation->GetZone() == color ) )
				continue;

			double sourceDistanceToTarget = c3ga::norm( sourceLocation->GetPosition() - targetLocation->GetPosition() );
			double destinationDistanceToTarget = c3ga::norm( destinationLocation->GetPosition() - targetLocation->GetPosition() );
			double moveDistance = c3ga::lc( moveDirection, generalMoveDirection );
			
			// Is this a move that gets us closer to the target?
			if( sourceDistanceToTarget > destinationDistanceToTarget )
			{
				// Special case: Do not consider lateral moves for pieces that are already in the target zone.
				if( fabs( projectedMoveDistance ) < epsilon && sourceLocation->GetZone() == zoneTarget )
					continue;

				// Yes.  Is it better than our current move?
				if( largestMoveDistance == -1.0 || moveDistance > largestMoveDistance )
				{
					// Yes.  Keep it.
					bestMove[0] = sourceID;
					bestMove[1] = destinationID;
					largestMoveDistance = moveDistance;
				}
			}
		}
	}

	// Free up all the memory we used.
	DeleteMoves( sourceMap );

	// Return our best move to the caller.
	sourceID = bestMove[0];
	destinationID = bestMove[1];
	if( sourceID == -1 || destinationID == -1 )
		return false;
	return true;
}

//=====================================================================================
Board::Location* Board::FindClosestUnoccupiedLocationInZone( const c3ga::vectorE3GA& position, int zone )
{
	Location* foundLocation = 0;
	double smallestDistance = -1.0;
	for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end(); iter++ )
	{
		Location* location = iter->second;
		if( location->GetZone() == zone && location->GetOccupant() == NONE )
		{
			double distance = c3ga::norm( location->GetPosition() - position );
			if( smallestDistance == -1.0 || distance < smallestDistance )
			{
				foundLocation = location;
				smallestDistance = distance;
			}
		}
	}
	return foundLocation;
}

//=====================================================================================
void Board::FindAllMovesForParticipant( int color, SourceMap& sourceMap )
{
	for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end(); iter++ )
	{
		Location* location = iter->second;
		if( location->GetOccupant() == color )
		{
			DestinationMap* destinationMap = FindAllMovesForLocation( location );
			sourceMap[ location->GetLocationID() ] = destinationMap;
		}
	}
}

//=====================================================================================
Board::DestinationMap* Board::FindAllMovesForLocation( Location* location )
{
	DestinationMap* destinationMap = new DestinationMap();

	for( int i = 0; i < ADJACENCIES; i++ )
	{
		Location* adjLocation = location->GetAdjacency(i);
		if( adjLocation && adjLocation->GetOccupant() == NONE )
			( *destinationMap )[ adjLocation->GetLocationID() ] = 1;
	}

	FindAllMovesForLocationRecursively( location, *destinationMap, 2 );

	return destinationMap;
}

//=====================================================================================
void Board::FindAllMovesForLocationRecursively( Location* location, DestinationMap& destinationMap, int edgeCount )
{
	location->Visited( true );

	for( int i = 0; i < ADJACENCIES; i++ )
	{
		Location* adjLocation = location->GetAdjacency(i);
		if( adjLocation && adjLocation->GetOccupant() != NONE )
		{
			Location* adjJumpLocation = adjLocation->GetAdjacency(i);
			if( adjJumpLocation && adjJumpLocation->GetOccupant() == NONE )
			{
				if( !adjJumpLocation->Visited() )
				{
					destinationMap[ adjJumpLocation->GetLocationID() ] = edgeCount;
					FindAllMovesForLocationRecursively( adjJumpLocation, destinationMap, edgeCount + 2 );
				}
			}
		}
	}

	location->Visited( false );
}

//=====================================================================================
void Board::DeleteMoves( SourceMap& sourceMap )
{
	for( SourceMap::iterator iter = sourceMap.begin(); iter != sourceMap.end(); iter++ )
	{
		DestinationMap* destinationMap = iter->second;
		delete destinationMap;
	}
	sourceMap.clear();
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

	return NONE;
}

//=====================================================================================
void Board::Render( GLenum renderMode, int highlightLocationID )
{
	if( renderMode == GL_SELECT )
		glPushName( -1 );

	for( LocationMap::iterator iter = locationMap.begin(); iter != locationMap.end(); iter++ )
	{
		Location* location = iter->second;
		bool highlight = false;
		if( location->GetLocationID() == highlightLocationID )
			highlight = true;
		location->Render( renderMode, highlight );
	}

	if( renderMode == GL_RENDER )
	{
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );
		glLightModelf( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
		glLightModelf( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );

		GLfloat lightPosition[] = { 10.f, 10.f, 10.f, 0.f };
		GLfloat lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.f };
		GLfloat lightSpecular[] = { 1.f, 1.f, 1.f, 1.f };
		GLfloat lightDiffuse[] = { 1.f, 1.f, 1.f, 1.f };
		glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
		glLightfv( GL_LIGHT0, GL_AMBIENT, lightAmbient );
		glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );
		glLightfv( GL_LIGHT0, GL_DIFFUSE, lightDiffuse );

		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		glFrontFace( GL_CCW );

		for( PieceList::iterator iter = pieceList.begin(); iter != pieceList.end(); iter++ )
		{
			Piece* piece = *iter;
			bool highlight = false;
			if( piece->GetLocationID() == highlightLocationID )
				highlight = true;
			piece->Render( this, highlight );
		}

		glDisable( GL_CULL_FACE );
		glDisable( GL_LIGHTING );
	}
}

//=====================================================================================
void Board::Animate( double frameRate )
{
	for( PieceList::iterator iter = pieceList.begin(); iter != pieceList.end(); iter++ )
	{
		Piece* piece = *iter;
		piece->Animate( frameRate );
	}
}

//=====================================================================================
bool Board::AnyPieceInMotion( void )
{
	for( PieceList::iterator iter = pieceList.begin(); iter != pieceList.end(); iter++ )
	{
		Piece* piece = *iter;
		if( piece->IsInMotion() )
			return true;
	}
	return false;
}

//=====================================================================================
int Board::FindSelectedLocation( unsigned int* hitBuffer, int hitBufferSize, int hitCount )
{
	int locationID = -1;

	// If the hit-count is -1 (indicating overflow), we don't process anything.
	unsigned int* hitRecord = hitBuffer;
	float smallestZ;
	for( int i = 0; i < hitCount; i++ )
	{
		unsigned int nameCount = hitRecord[0];
		float minZ = float( hitRecord[1] ) / float( 0x7FFFFFFFF );
		if( nameCount == 1 )
		{
			if( locationID == -1 || minZ < smallestZ )
			{
				smallestZ = minZ;
				locationID = ( signed )hitRecord[3];
			}
		}
		hitRecord += 3 + nameCount;
	}

	return locationID;
}

//=====================================================================================
bool Board::FindMoveSequence( int sourceID, int destinationID, MoveSequence& moveSequence )
{
	moveSequence.clear();

	LocationMap::iterator iter = locationMap.find( sourceID );
	if( iter == locationMap.end() )
		return false;
	Location* sourceLocation = iter->second;

	iter = locationMap.find( destinationID );
	if( iter == locationMap.end() )
		return false;
	Location* destinationLocation = iter->second;

	// As part of the definition of a valid move, only the player who's turn it is can make the move.
	if( sourceLocation->GetOccupant() != whosTurn )
		return false;

	// A piece can only be moved to an unoccupied location.
	if( destinationLocation->GetOccupant() != NONE )
		return false;

	// A sequence of one move can always be performed to an immediate adjacency.
	for( int i = 0; i < ADJACENCIES; i++ )
	{
		Location* adjLocation = sourceLocation->GetAdjacency(i);
		if( adjLocation == destinationLocation )
		{
			moveSequence.push_back( sourceID );
			moveSequence.push_back( destinationID );
			return true;
		}
	}

	// If the destination is further away, then it can be achieved as a sequence of jumps over existing pieces.
	return FindMoveSequenceRecursively( sourceLocation, destinationLocation, moveSequence );
}

//=====================================================================================
// Assuming a valid path between the two given locations exists, this algorithm
// returns the first one it finds, which may not be the shortest such path.
bool Board::FindMoveSequenceRecursively( Location* currentLocation, Location* destinationLocation, MoveSequence& moveSequence )
{
	bool found = false;
	currentLocation->Visited( true );

	// Have we found our destination yet?
	if( currentLocation == destinationLocation )
		found = true;
	else
	{
		// No, go search for it in all directions.
		for( int i = 0; i < ADJACENCIES && !found; i++ )
		{
			// Can we jump in this direction?
			Location* adjLocation = currentLocation->GetAdjacency(i);
			if( !adjLocation || adjLocation->GetOccupant() == NONE )
				continue;
			Location* adjJumpLocation = adjLocation->GetAdjacency(i);
			if( !adjJumpLocation || adjJumpLocation->GetOccupant() != NONE )
				continue;
				
			// Yes, we can, but have we already searched from the location we're going to jump to?
			if( adjJumpLocation->Visited() )
				continue;
			
			// No, we haven't, so go try it.
			found = FindMoveSequenceRecursively( adjJumpLocation, destinationLocation, moveSequence );
		}
	}

	// If we find the destination, build up the sequence.
	if( found )
		moveSequence.push_front( currentLocation->GetLocationID() );

	currentLocation->Visited( false );
	return found;
}

//=====================================================================================
// Here we assume that the given move sequence is valid.
bool Board::ApplyMoveSequence( const MoveSequence& moveSequence )
{
	int sourceID = *moveSequence.begin();
	int destinationID = moveSequence.back();

	LocationMap::iterator iter = locationMap.find( sourceID );
	Location* sourceLocation = iter->second;
	iter = locationMap.find( destinationID );
	Location* destinationLocation = iter->second;

	destinationLocation->SetOccupant( sourceLocation->GetOccupant() );
	sourceLocation->SetOccupant( NONE );

	Piece* piece = sourceLocation->GetPiece();
	destinationLocation->SetPiece( piece );
	sourceLocation->SetPiece(0);
	if( piece )
		piece->ResetAnimation( moveSequence );

	NextTurn();

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
	visited = false;

	for( int i = 0; i < ADJACENCIES; i++ )
		adjacency[i] = 0;
}

//=====================================================================================
Board::Location::~Location( void )
{
}

//=====================================================================================
void Board::Location::Visited( bool visited )
{
	this->visited = visited;
}

//=====================================================================================
bool Board::Location::Visited( void )
{
	return visited;
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
		
		if( visitationMap.end() != visitationMap.find( adjLocation->locationID ) )
			continue;

		double angle = double(i) / double( ADJACENCIES ) * 2.0 * M_PI;
		double radius = double( LOCATION_EDGE_LENGTH );
		c3ga::vectorE3GA adjPosition;
		adjPosition.set( c3ga::vectorE3GA::coord_e1_e2_e3, radius * cos( angle ), 0.0, radius * sin( angle ) );
		adjPosition = position + adjPosition;
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
int Board::Location::GetLocationID( void )
{
	return locationID;
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
void Board::Location::SetPiece( Piece* piece )
{
	this->piece = piece;
}

//=====================================================================================
Board::Piece* Board::Location::GetPiece( void )
{
	return piece;
}

//=====================================================================================
const c3ga::vectorE3GA& Board::Location::GetPosition( void )
{
	return position;
}

//=====================================================================================
int Board::Location::GetAdjacencyCount( void )
{
	int count = 0;
	for( int i = 0; i < ADJACENCIES; i++ )
		if( adjacency[i] )
			count++;
	return count;
}

//=====================================================================================
void Board::Location::GetRenderColor( c3ga::vectorE3GA& renderColor )
{
	RenderColor( zone, renderColor );
}

//=====================================================================================
/*static*/ void Board::RenderColor( int color, c3ga::vectorE3GA& renderColor )
{
	// This array depends upon the enum order in the header file!
	static GLfloat locationColorMap[ CYAN + 1 ][3] =
	{
		{ 1.f, 1.f, 1.f },			// NONE (white)
		{ 1.f, 0.f, 0.f },			// RED
		{ 0.f, 1.f, 0.f },			// GREEN
		{ 0.f, 0.f, 1.f },			// BLUE
		{ 1.f, 1.f, 0.f },			// YELLOW
		{ 1.f, 0.f, 1.f },			// MAGENTA
		{ 0.f, 1.f, 1.f },			// CYAN
	};

	renderColor.set( c3ga::vectorE3GA::coord_e1_e2_e3,
							locationColorMap[ color ][0],
							locationColorMap[ color ][1],
							locationColorMap[ color ][2] );
}

//=====================================================================================
void Board::Location::Render( GLenum renderMode, bool highlight )
{
	if( renderMode == GL_RENDER )
	{
		c3ga::vectorE3GA renderColor;
		GetRenderColor( renderColor );

		glBegin( GL_LINES );
		glLineWidth( 2.f );

		for( int i = 0; i < ADJACENCIES; i++ )
		{
			Location* adjLocation = adjacency[i];
			if( adjLocation )
			{
				c3ga::vectorE3GA adjRenderColor;
				adjLocation->GetRenderColor( adjRenderColor );

				double t = 0.55;
				c3ga::vectorE3GA midPoint = c3ga::add( c3ga::gp( position, 1.0 - t ), c3ga::gp( adjLocation->position, t ) );
				c3ga::vectorE3GA midColor = c3ga::add( c3ga::gp( renderColor, 1.0 - t ), c3ga::gp( adjRenderColor, t ) );

				glColor3f( renderColor.get_e1(), renderColor.get_e2(), renderColor.get_e3() );
				glVertex3f( position.get_e1(), position.get_e2(), position.get_e3() );
				glColor3f( midColor.get_e1(), midColor.get_e2(), midColor.get_e3() );
				glVertex3f( midPoint.get_e1(), midPoint.get_e2(), midPoint.get_e3() );
			}
		}

		glEnd();
	}
	else if( renderMode == GL_SELECT )
	{
		// In this case, we want to render a hot-spot hexagon.
		glLoadName( locationID );
		glBegin( GL_POLYGON );
		glColor3f( 1.f, 1.f, 1.f );
		double radius = double( LOCATION_EDGE_LENGTH ) * 0.5;
		for( int i = 0; i < ADJACENCIES; i++ )
		{
			double angle = double(i) / double( ADJACENCIES ) * 2.0 * M_PI;
			double x = position.get_e1() + radius * cos( angle );
			double z = position.get_e3() + radius * sin( angle );
			glVertex3f( x, 0.f, z );
		}
		glEnd();
	}
}

//=====================================================================================
Board::Piece::Piece( int locationID )
{
	// At rest, the piece has just one location in its move sequence.
	moveSequence.push_back( locationID );

	pivotAngle = 0.0;
}

//=====================================================================================
Board::Piece::~Piece( void )
{
}

//=====================================================================================
int Board::Piece::GetLocationID( void )
{
	if( moveSequence.size() == 0 )
		return -1;

	int locationID = moveSequence.back();
	return locationID;
}

//=====================================================================================
void Board::Piece::Render( Board* board, bool highlight )
{
	// This should never be the case, but protect against it anyway.
	if( moveSequence.size() == 0 )
		return;

	int locationID = GetLocationID();
	int occupant = board->OccupantAtLocation( locationID );

	c3ga::vectorE3GA piecePosition;
	if( moveSequence.size() == 1 )
		board->PositionAtLocation( locationID, piecePosition );
	else
	{
		MoveSequence::iterator iter = moveSequence.begin();
		int prevLocationID = *iter++;
		int nextLocationID = *iter;
		c3ga::vectorE3GA prevPosition, nextPosition;
		board->PositionAtLocation( prevLocationID, prevPosition );
		board->PositionAtLocation( nextLocationID, nextPosition );

		c3ga::vectorE3GA upVector( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 1.0, 0.0 );
		c3ga::bivectorE3GA blade = c3ga::op( c3ga::unit( nextPosition - prevPosition ), upVector );
		c3ga::rotorE3GA rotor = c3ga::exp( c3ga::gp( blade, 0.5 * pivotAngle ) );

		c3ga::vectorE3GA midPoint = c3ga::gp( prevPosition, 0.5 ) + c3ga::gp( nextPosition, 0.5 );
		c3ga::vectorE3GA pivotVector = prevPosition - midPoint;
		pivotVector = c3ga::applyUnitVersor( rotor, pivotVector );
		piecePosition = midPoint + pivotVector;
	}
	
	c3ga::vectorE3GA renderColor;
	RenderColor( occupant, renderColor );
	
	if( glIsEnabled( GL_LIGHTING ) )
	{
		GLfloat specularity[] = { 1.f, 1.f, 1.f, 1.f };
		GLfloat shininess[] = { 50.f };
		GLfloat ambientDiffuse[] = { renderColor.get_e1(), renderColor.get_e2(), renderColor.get_e3(), 1.f };
		GLfloat emissive[] = { 0.f, 0.f, 0.f, 0.f };

		if( highlight )
		{
			emissive[0] = 0.5f;
			emissive[1] = 0.5f;
			emissive[2] = 0.5f;
			emissive[3] = 1.f;
		}

		glMaterialfv( GL_FRONT, GL_SPECULAR, specularity );
		glMaterialfv( GL_FRONT, GL_SHININESS, shininess );
		glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambientDiffuse );
		glMaterialfv( GL_FRONT, GL_EMISSION, emissive );
	}
	else
	{
		if( highlight )
			glColor3f( 1.f, 1.f, 1.f );
		else
			glColor3f( renderColor.get_e1(), renderColor.get_e2(), renderColor.get_e3() );
	}

	double radius = double( LOCATION_EDGE_LENGTH ) * 0.4;
	Sphere::Render( piecePosition, radius, 1 );
}

//=====================================================================================
// This is called once per frame to animate a board piece.
void Board::Piece::Animate( double frameRate )
{
	if( moveSequence.size() > 1 )
	{
		double radiansPerFrame = ROTATION_RATE * ( M_PI / 180.0 ) / frameRate;
		pivotAngle += radiansPerFrame;
		if( pivotAngle > M_PI )
		{
			pivotAngle = 0.0;
			moveSequence.pop_front();
		}
	}
}

//=====================================================================================
bool Board::Piece::IsInMotion( void )
{
	if( moveSequence.size() > 1 )
		return true;
	return false;
}

//=====================================================================================
void Board::Piece::ResetAnimation( const MoveSequence& moveSequence )
{
	pivotAngle = 0.0;
	this->moveSequence = moveSequence;
}

// ChiCheBoard.cpp