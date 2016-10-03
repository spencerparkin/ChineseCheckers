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
	FreeCacheMap();
}

//=====================================================================================
bool Brain::FindGoodMoveForParticipant( int color, Board* board, Board::Move& move )
{
	Cache* cache = nullptr;

	CacheMap::iterator iter = cacheMap.find( color );
	if( iter != cacheMap.end() )
	{
		cache = iter->second;
		if( !cache->IsValid( board ) )
		{
			delete cache;
			cache = nullptr;
			cacheMap.erase( iter );
		}
		else
		{
			// Sometimes it's dumb to use the cache if the new state of the game
			// is such that the move we make stops short of how far we could go.
			cache->MakeNextMove( move );

			// TODO: See if the move can be improved here?  An improvement may or
			//       may not invalidate the cache on our next turn.

			return true;
		}
	}

	if( !cache )
		cache = new Cache();
	
	Board::Location* targetVertexLocation = nullptr;
	Board::Location* sourceVertexLocation = nullptr;

	for( Board::LocationMap::iterator iter = board->locationMap.begin(); iter != board->locationMap.end(); iter++ )
	{
		Board::Location* location = iter->second;

		if( location->GetAdjacencyCount() == 2 )
		{
			if( location->GetZone() == color )
				sourceVertexLocation = location;
			else if( location->GetZone() == board->ZoneTarget( color ) )
				targetVertexLocation = location;
		}
	}

	wxASSERT( targetVertexLocation && sourceVertexLocation );

	GeneralMetrics generalMetrics;
	generalMetrics.generalMoveDir = c3ga::unit( targetVertexLocation->GetPosition() - sourceVertexLocation->GetPosition() );

	int maxMoveCount = 2;

	Board::MoveList moveList;
	ExamineEveryOutcomeForBestMoveSequence( color, board, generalMetrics, moveList, maxMoveCount, cache );

	cache->MakeNextMove( move );
	return true;
}

//=====================================================================================
void Brain::ExamineEveryOutcomeForBestMoveSequence( int color, Board* board, const GeneralMetrics& generalMetrics, Board::MoveList& moveList, int maxMoveCount, Cache*& cache )
{
	int winningColor = board->DetermineWinner();
	
	if( moveList.size() == maxMoveCount || winningColor == color )
	{
		Cache* newCache = new Cache( &moveList );

		if( cache->Compare( color, board, generalMetrics, newCache ) )
		{
			delete cache;
			cache = newCache;
		}
		else
		{
			delete newCache;
			newCache = nullptr;
		}
	}
	else
	{
		Board::LocationList sourceLocationList;
		board->FindParticpantLocations( color, sourceLocationList );

		for( Board::LocationList::iterator srcIter = sourceLocationList.begin(); srcIter != sourceLocationList.end(); srcIter++ )
		{
			Board::Location* sourceLocation = *srcIter;

			Board::LocationList destinationLocationList;
			board->FindAllPossibleDestinations( sourceLocation, destinationLocationList );

			for( Board::LocationList::iterator dstIter = destinationLocationList.begin(); dstIter != destinationLocationList.end(); dstIter++ )
			{
				Board::Location* destinationLocation = *dstIter;

				Board::Move move;
				move.sourceID = sourceLocation->GetLocationID();
				move.destinationID = destinationLocation->GetLocationID();

				bool applied = board->ApplyMoveInternally( move );
				wxASSERT( applied );

				moveList.push_back( move );

				ExamineEveryOutcomeForBestMoveSequence( color, board, generalMetrics, moveList, maxMoveCount, cache );

				moveList.pop_back();

				Board::Move invMove;
				move.Inverse( invMove );

				applied = board->ApplyMoveInternally( invMove );
				wxASSERT( applied );
			}
		}
	}
}

//=====================================================================================
Brain::Cache::Cache( Board::MoveList* moveList /*= nullptr*/ )
{
	metrics = nullptr;

	SetMoveList( moveList );
}

//=====================================================================================
/*virtual*/ Brain::Cache::~Cache( void )
{
	delete metrics;
}

//=====================================================================================
void Brain::Cache::SetMoveList( Board::MoveList* moveList )
{
	this->moveList.clear();

	if( moveList )
	{
		Board::MoveList::iterator iter = moveList->begin();
		while( iter != moveList->end() )
		{
			this->moveList.push_back( *iter );
			iter++;
		}
	}
}

//=====================================================================================
bool Brain::Cache::IsValid( Board* board )
{
	if( moveList.size() == 0 )
		return false;

	Board::MoveList::iterator iter = moveList.begin();
	if( !RecursivelyValidateMoveList( board, iter ) )
		return false;

	return true;
}

//=====================================================================================
bool Brain::Cache::RecursivelyValidateMoveList( Board* board, Board::MoveList::iterator& iter )
{
	bool valid = true;
	Board::Move move = *iter;

	if( !board->ApplyMoveInternally( move ) )
		valid = false;
	else
	{
		iter++;
		if( !RecursivelyValidateMoveList( board, iter ) )
			valid = false;

		Board::Move invMove;
		move.Inverse( invMove );
		bool applied = board->ApplyMoveInternally( invMove );
		wxASSERT( applied );
	}

	return valid;
}

//=====================================================================================
bool Brain::Cache::MakeNextMove( Board::Move& move )
{
	if( moveList.size() == 0 )
		return false;

	move = *moveList.begin();
	moveList.pop_front();

	return true;
}

//=====================================================================================
bool Brain::Cache::Compare( int color, Board* board, const GeneralMetrics& generalMetrics, Cache* cache )
{
	Metrics* thisMetrics = GetMetrics( color, board, generalMetrics );
	Metrics* otherMetrics = cache->GetMetrics( color, board, generalMetrics );

	if( otherMetrics->targetZoneLandingCount > thisMetrics->targetZoneLandingCount )
		return true;

	if( otherMetrics->netProjectedSignedDistance > thisMetrics->netProjectedSignedDistance )
		return true;

	return false;
}

//=====================================================================================
Brain::Cache::Metrics* Brain::Cache::GetMetrics( int color, Board* board, const GeneralMetrics& generalMetrics )
{
	if( !metrics )
	{
		metrics = new Metrics();

		metrics->netProjectedSignedDistance = 0.0;

		for( Board::MoveList::iterator iter = moveList.begin(); iter != moveList.end(); iter++ )
		{
			const Board::Move& move = *iter;
			
			c3ga::vectorE3GA sourcePosition, destinationPosition;

			board->PositionAtLocation( move.sourceID, sourcePosition );
			board->PositionAtLocation( move.destinationID, destinationPosition );

			c3ga::vectorE3GA moveVector = destinationPosition - sourcePosition;
			double projectedSignedDistance = c3ga::lc( moveVector, generalMetrics.generalMoveDir );
			metrics->netProjectedSignedDistance += projectedSignedDistance;
		}

		metrics->targetZoneLandingCount = 0;

		int zoneTarget = board->ZoneTarget( color );

		for( Board::MoveList::iterator iter = moveList.begin(); iter != moveList.end(); iter++ )
		{
			const Board::Move& move = *iter;

			Board::Location* sourceLocation = board->locationMap[ move.sourceID ];
			Board::Location* destinationLocation = board->locationMap[ move.destinationID ];

			if( sourceLocation->GetZone() != zoneTarget && destinationLocation->GetZone() == zoneTarget )
				metrics->targetZoneLandingCount++;
			else if( sourceLocation->GetZone() == zoneTarget && destinationLocation->GetZone() != zoneTarget )
				metrics->targetZoneLandingCount--;
		}
	}

	return metrics;
}

//=====================================================================================
void Brain::FreeCacheMap( void )
{
	while( cacheMap.size() > 0 )
	{
		CacheMap::iterator iter = cacheMap.begin();
		Cache* cache = iter->second;
		delete cache;
		cacheMap.erase( iter );
	}
}

// ChiCheBrain.cpp