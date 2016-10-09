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
			cache->MakeNextMove( move );

			// Sometimes a move is optimal at the time of its conception, but becomes sub-optimal
			// at the time we're about to execute it.  So here we see if the given move can be
			// made to be a little bit better.
			ImproveMove( color, board, move );

			return true;
		}
	}

	if( !cache )
		cache = new Cache();

	GeneralMetrics generalMetrics;
	generalMetrics.targetCentroid.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 0.0, 0.0 );
	double targetCount = 0.0;
	int targetZone = board->ZoneTarget( color );

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

		if( location->GetZone() == targetZone && location->GetOccupant() == Board::NONE )
		{
			generalMetrics.targetCentroid += location->GetPosition();
			targetCount += 1.0;
		}
	}

	wxASSERT( targetVertexLocation && sourceVertexLocation );
	
	if( targetCount == 0.0 )
		targetCount = 10.0;

	generalMetrics.targetCentroid *= 1.0 / targetCount;
	generalMetrics.generalMoveDir = c3ga::unit( targetVertexLocation->GetPosition() - sourceVertexLocation->GetPosition() );

	int maxMoveCount = 2;

	Board::MoveList moveList;
	ExamineEveryOutcomeForBestMoveSequence( color, board, generalMetrics, moveList, maxMoveCount, cache );

	cache->MakeNextMove( move );

	cacheMap.insert( std::pair< int, Cache* >( color, cache ) );

	return true;
}

//=====================================================================================
void Brain::ImproveMove( int color, Board* board, Board::Move& move )
{
	Board::Location* sourceLocation = board->locationMap[ move.sourceID ];
	Board::Location* destinationLocation = board->locationMap[ move.destinationID ];

	// Single-jump moves can't be improved.
	for( int i = 0; i < Board::ADJACENCIES; i++ )
		if( sourceLocation->GetAdjacency(i) == destinationLocation )
			return;

	Board::Location* targetVertexLocation = nullptr;
	for( Board::LocationMap::iterator iter = board->locationMap.begin(); iter != board->locationMap.end(); iter++ )
	{
		Board::Location* location = iter->second;
		if( location->GetAdjacencyCount() == 2 && location->GetZone() == board->ZoneTarget( color ) )
		{
			targetVertexLocation = location;
			break;
		}
	}

	wxASSERT( targetVertexLocation );

	ImproveMoveRecursively( destinationLocation, targetVertexLocation, destinationLocation );

	move.destinationID = destinationLocation->GetLocationID();
}

//=====================================================================================
void Brain::ImproveMoveRecursively( Board::Location* location, Board::Location* targetVertexLocation, Board::Location*& bestLocation )
{
	location->Visited( true );

	double bestDistance = c3ga::norm( bestLocation->GetPosition() - targetVertexLocation->GetPosition() );
	double distance = c3ga::norm( location->GetPosition() - targetVertexLocation->GetPosition() );
	if( distance < bestDistance )
		bestLocation = location;

	for( int i = 0; i < Board::ADJACENCIES; i++ )
	{
		Board::Location* adjacentLocation = location->GetAdjacency(i);
		if( adjacentLocation && adjacentLocation->GetOccupant() != Board::NONE )
		{
			adjacentLocation = adjacentLocation->GetAdjacency(i);
			if( adjacentLocation && adjacentLocation->GetOccupant() == Board::NONE )
			{
				if( !adjacentLocation->Visited() )
					ImproveMoveRecursively( adjacentLocation, targetVertexLocation, bestLocation );
			}
		}
	}

	location->Visited( false );
}

//=====================================================================================
void Brain::ExamineEveryOutcomeForBestMoveSequence( int color, Board* board, const GeneralMetrics& generalMetrics, Board::MoveList& moveList, int maxMoveCount, Cache*& cache )
{
	// TODO: I see the computer lose most often when it leaves a straggler behind.
	//       If we detect a straggler, we might restrict our set of considered moves
	//       to only those that move the straggler forward.

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
				int zone = destinationLocation->GetZone();
				if( !( zone == Board::NONE || zone == color || zone == board->ZoneTarget( color ) ) )
					continue;

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

	Board::MoveSequence moveSequence;
	if( !board->FindMoveSequence( move, moveSequence ) )
		valid = false;
	else
	{
		bool applied = board->ApplyMoveInternally( move );
		wxASSERT( applied );
	
		iter++;
		if( iter != moveList.end() )
			if( !RecursivelyValidateMoveList( board, iter ) )
				valid = false;

		Board::Move invMove;
		move.Inverse( invMove );
		applied = board->ApplyMoveInternally( invMove );
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
	if( moveList.size() == 0 && cache->moveList.size() > 0 )
		return true;
	if( cache->moveList.size() == 0 )
		return false;

	Metrics* thisMetrics = GetMetrics( color, board, generalMetrics );
	Metrics* otherMetrics = cache->GetMetrics( color, board, generalMetrics );

	if( otherMetrics->targetZoneLandingCount > thisMetrics->targetZoneLandingCount )
		return true;
	if( otherMetrics->targetZoneLandingCount < thisMetrics->targetZoneLandingCount )
		return false;

	double eps = 1e-4;

	if( fabs( otherMetrics->netProjectedSignedDistance - thisMetrics->netProjectedSignedDistance ) >= eps )
	{
		if( otherMetrics->netProjectedSignedDistance > thisMetrics->netProjectedSignedDistance )
			return true;
		if( otherMetrics->netProjectedSignedDistance < thisMetrics->netProjectedSignedDistance )
			return false;
	}

	if( fabs( otherMetrics->totalDistanceToTargetCentroid - thisMetrics->totalDistanceToTargetCentroid ) >= eps )
	{
		if( otherMetrics->totalDistanceToTargetCentroid < thisMetrics->totalDistanceToTargetCentroid )
			return true;
		if( otherMetrics->totalDistanceToTargetCentroid > thisMetrics->totalDistanceToTargetCentroid )
			return false;
	}

	if( cache->moveList.size() < moveList.size() )
		return true;
	if( cache->moveList.size() > moveList.size() )
		return false;

	// Here, the return value is arbitrary.  We might return a random boolean to keep things interesting.
	return true;
}

//=====================================================================================
Brain::Cache::Metrics* Brain::Cache::GetMetrics( int color, Board* board, const GeneralMetrics& generalMetrics )
{
	if( !metrics )
	{
		metrics = new Metrics();

		metrics->netProjectedSignedDistance = 0.0;
		metrics->targetZoneLandingCount = 0;
		metrics->totalDistanceToTargetCentroid = 0.0;

		int zoneTarget = board->ZoneTarget( color );

		for( Board::MoveList::iterator iter = moveList.begin(); iter != moveList.end(); iter++ )
		{
			const Board::Move& move = *iter;
			
			Board::Location* sourceLocation = board->locationMap[ move.sourceID ];
			Board::Location* destinationLocation = board->locationMap[ move.destinationID ];

			const c3ga::vectorE3GA& sourcePosition = sourceLocation->GetPosition();
			const c3ga::vectorE3GA& destinationPosition = destinationLocation->GetPosition();

			c3ga::vectorE3GA moveVector = destinationPosition - sourcePosition;
			double projectedSignedDistance = c3ga::lc( moveVector, generalMetrics.generalMoveDir );
			metrics->netProjectedSignedDistance += projectedSignedDistance;

			if( sourceLocation->GetZone() != zoneTarget && destinationLocation->GetZone() == zoneTarget )
				metrics->targetZoneLandingCount++;
			else if( sourceLocation->GetZone() == zoneTarget && destinationLocation->GetZone() != zoneTarget )
				metrics->targetZoneLandingCount--;

			double distanceToTargetCentroid = c3ga::norm( destinationPosition - generalMetrics.targetCentroid );
			metrics->totalDistanceToTargetCentroid += distanceToTargetCentroid;
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