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
void Brain::CalculateGeneralMetrics( int color, Board* board, GeneralMetrics& generalMetrics )
{
	generalMetrics.targetCentroid.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, 0.0, 0.0 );
	double targetCount = 0.0;
	int targetZone = board->ZoneTarget( color );

	generalMetrics.targetVertexLocation = nullptr;
	generalMetrics.sourceVertexLocation = nullptr;

	for( Board::LocationMap::iterator iter = board->locationMap.begin(); iter != board->locationMap.end(); iter++ )
	{
		Board::Location* location = iter->second;

		if( location->GetAdjacencyCount() == 2 )
		{
			if( location->GetZone() == color )
				generalMetrics.sourceVertexLocation = location;
			else if( location->GetZone() == board->ZoneTarget( color ) )
				generalMetrics.targetVertexLocation = location;
		}

		if( location->GetZone() == targetZone && location->GetOccupant() == Board::NONE )
		{
			generalMetrics.targetCentroid += location->GetPosition();
			targetCount += 1.0;
		}
	}

	wxASSERT( generalMetrics.targetVertexLocation && generalMetrics.sourceVertexLocation );

	if( targetCount == 0.0 )
		targetCount = 10.0;

	c3ga::vectorE3GA moveAxis = generalMetrics.targetVertexLocation->GetPosition() - generalMetrics.sourceVertexLocation->GetPosition();

	generalMetrics.targetCentroid *= 1.0 / targetCount;
	generalMetrics.generalMoveDir = c3ga::unit( moveAxis );

	generalMetrics.stragglerLocation = nullptr;
	generalMetrics.leaderLocation = nullptr;

	double smallestProjectedDistance = 0.0;
	double largestProjectedDistance = 0.0;

	Board::LocationList locationList;
	board->FindParticpantLocations( color, locationList );
	for( Board::LocationList::iterator iter = locationList.begin(); iter != locationList.end(); iter++ )
	{
		Board::Location* location = *iter;

		double projectedDistance = c3ga::lc( generalMetrics.generalMoveDir, location->GetPosition() - generalMetrics.sourceVertexLocation->GetPosition() );

		if( !generalMetrics.stragglerLocation || projectedDistance < smallestProjectedDistance )
		{
			smallestProjectedDistance = projectedDistance;
			generalMetrics.stragglerLocation = location;
		}

		if( !generalMetrics.leaderLocation || projectedDistance > largestProjectedDistance )
		{
			largestProjectedDistance = projectedDistance;
			generalMetrics.leaderLocation = location;
		}
	}
}

//=====================================================================================
// TODO: There are still some really stupid moves being made by the computer near the end-game.
//       Losing due to straggling is still a problem, and a straggler can even get completely
//       stuck inside the opponent's zone.  Another thought I had is that a generated move sequence
//       might be something that should be sorted: best move to worst.  This, of course, would have
//       have to be done under the constraint that some moves don't communite, because one may be
//       dependent upon the other.  But the idea is to get more bang for our buck while the board
//       is still in our favor.
bool Brain::FindGoodMoveForParticipant( int color, Board* board, Board::Move& move )
{
	GeneralMetrics generalMetrics;
	CalculateGeneralMetrics( color, board, generalMetrics );

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
			ImproveMove( color, board, move, generalMetrics );

			return true;
		}
	}

	if( !cache )
		cache = new Cache();

	// This is how many moves ahead we're thinking.  Now, increasing this wouldn't necessarily be a good thing,
	// not just because it's slow, but because the further out we go, the less likely the board state is going
	// to be what we want it to be.  In other words, the cache would almost certainly become invalid before long anyway.
	int maxMoveCount = 3;
	Board::MoveList moveList;
	ExamineEveryOutcomeForBestMoveSequenceOnMultipleThreads( color, board, generalMetrics, moveList, maxMoveCount, cache );

	if( !cache->MakeNextMove( move ) )
	{
		delete cache;
		return false;
	}

	cacheMap.insert( std::pair< int, Cache* >( color, cache ) );
	return true;
}

//=====================================================================================
void Brain::ImproveMove( int color, Board* board, Board::Move& move, const GeneralMetrics& generalMetrics )
{
	Board::Location* sourceLocation = board->locationMap[ move.sourceID ];
	Board::Location* destinationLocation = board->locationMap[ move.destinationID ];

	// Single-jump moves can't be improved.
	for( int i = 0; i < Board::ADJACENCIES; i++ )
		if( sourceLocation->GetAdjacency(i) == destinationLocation )
			return;

	ImproveMoveRecursively( destinationLocation, generalMetrics.targetVertexLocation, destinationLocation );

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
// I'm sure there's a faster threading model, but if this brings us down to seconds from minutes, I'm happy with that.
/*static*/ void Brain::ExamineEveryOutcomeForBestMoveSequenceOnMultipleThreads( int color, Board* board, const GeneralMetrics& generalMetrics, Board::MoveList& moveList, int maxMoveCount, Cache*& cache )
{
	int cpuCount = wxThread::GetCPUCount();		// Does this count cores?
	if( maxMoveCount <= 2 || cpuCount <= 1 )
		return ExamineEveryOutcomeForBestMoveSequence( color, board, generalMetrics, moveList, maxMoveCount, cache, -1 );

	Board::LocationList sourceLocationList;
	board->FindParticpantLocations( color, sourceLocationList );

	ThreadList threadList;
	for( int i = 0; i < cpuCount; i++ )
	{
		Thread* thread = new Thread( color, board, &generalMetrics, maxMoveCount );
		if( thread->Run() == wxTHREAD_NO_ERROR )
			threadList.push_back( thread );
		else
			delete thread;
	}

	int sourceCount = sourceLocationList.size();
	wxProgressDialog* progressDialog = new wxProgressDialog( "Chinese Checkers", "Thinking...", sourceCount, wxGetApp().GetFrame(), wxPD_APP_MODAL | wxPD_AUTO_HIDE );

	// Until we've processed all locations...
	while( sourceLocationList.size() > 0 )
	{
		// If a thread has a result ready, process that result.
		for( ThreadList::iterator iter = threadList.begin(); iter != threadList.end(); iter++ )
		{
			Thread* thread = *iter;
			wxMutexLocker mutexLocker( thread->mutex );
			if( thread->state == Thread::STATE_RESULT_READY )
			{
				cache = Brain::ChooseBetweenCaches( color, board, generalMetrics, cache, thread->cache );
				thread->cache = nullptr;
				thread->state = Thread::STATE_READY_FOR_WORK;
			}
		}

		// Grab a location that needs to be processed.
		Board::LocationList::iterator locationIter = sourceLocationList.begin();
		Board::Location* sourceLocation = *locationIter;

		// Try to find a thread to process it.
		Thread* readyThread = nullptr;
		for( ThreadList::iterator iter = threadList.begin(); iter != threadList.end(); iter++ )
		{
			Thread* thread = *iter;
			wxMutexLocker mutexLocker( thread->mutex );
			if( thread->state == Thread::STATE_READY_FOR_WORK )
			{
				readyThread = thread;
				break;
			}
		}

		// All threads are busy, try again later.
		if( !readyThread )
			wxSleep(1);
		else
		{
			// Off-load work to the thread.
			wxMutexLocker mutexLocker( readyThread->mutex );
			readyThread->sourceID = sourceLocation->GetLocationID();
			readyThread->state = Thread::STATE_WORKING;
			readyThread->cache = new Cache();
			sourceLocationList.erase( locationIter );

			progressDialog->Update( sourceCount - sourceLocationList.size(),
					wxString::Format( "Thinking... %d/%d", sourceCount - sourceLocationList.size(), sourceCount ) );
		}
	}

	// Shutdown all the threads.
	while( threadList.size() > 0 )
	{
		ThreadList::iterator iter = threadList.begin();
		Thread* thread = *iter;
		wxMutexLocker( thread->mutex );
		thread->state = Thread::STATE_DYING;
		threadList.erase( iter );
	}

	delete progressDialog;
}

//=====================================================================================
Brain::Thread::Thread( int color, const Board* board, const GeneralMetrics* generalMetrics, int maxMoveCount ) : wxThread( wxTHREAD_DETACHED )
{
	state = STATE_READY_FOR_WORK;
	this->board = board->Clone();
	this->color = color;
	this->generalMetrics = generalMetrics;
	this->maxMoveCount = maxMoveCount;
	sourceID = -1;
	cache = nullptr;
}

//=====================================================================================
/*virtual*/ Brain::Thread::~Thread( void )
{
}

//=====================================================================================
/*virtual*/ void* Brain::Thread::Entry( void )
{
	while( state != STATE_DYING )
	{
		{
			wxMutexLocker mutexLocker( mutex );
			if( state != STATE_WORKING )
			{
				wxSleep(1);
				continue;
			}
		}

		Board::MoveList moveList;
		Brain::ExamineEveryOutcomeForBestMoveSequence( color, board, *generalMetrics, moveList, maxMoveCount, cache, sourceID );

		{
			wxMutexLocker mutexLocker( mutex );
			if( state == STATE_WORKING )
				state = STATE_RESULT_READY;
		}
	}

	return 0;
}

//=====================================================================================
/*static*/ Brain::Cache* Brain::ChooseBetweenCaches( int color, Board* board, const GeneralMetrics& generalMetrics, Cache* cacheA, Cache* cacheB )
{
	if( cacheA->Compare( color, board, generalMetrics, cacheB ) )
	{
		delete cacheA;
		return cacheB;
	}

	delete cacheB;
	return cacheA;
}

//=====================================================================================
/*static*/ void Brain::ExamineEveryOutcomeForBestMoveSequence( int color, Board* board, const GeneralMetrics& generalMetrics, Board::MoveList& moveList, int maxMoveCount, Cache*& cache, int sourceID, bool moveSourceOnly /*= false*/ )
{
	int winningColor = board->DetermineWinner();
	
	if( moveList.size() == maxMoveCount || winningColor == color )
	{
		Cache* newCache = new Cache( &moveList );
		cache = ChooseBetweenCaches( color, board, generalMetrics, cache, newCache );
	}
	else
	{
		Board::LocationList sourceLocationList;
		if( sourceID >= 0 )
			sourceLocationList.push_back( board->locationMap[ sourceID ] );
		else
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

				ExamineEveryOutcomeForBestMoveSequence( color, board, generalMetrics, moveList, maxMoveCount, cache, ( ( sourceID >= 0 && moveSourceOnly ) ? move.destinationID : -1 ) );

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

	if( fabs( otherMetrics->netDeltaToTargetCentroid - thisMetrics->netDeltaToTargetCentroid ) >= eps )
	{
		if( otherMetrics->netDeltaToTargetCentroid < thisMetrics->netDeltaToTargetCentroid )
			return true;
		if( otherMetrics->netDeltaToTargetCentroid > thisMetrics->netDeltaToTargetCentroid )
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
		metrics->netDeltaToTargetCentroid = 0.0;

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

			double preDistanceToTargetCentroid = c3ga::norm( sourcePosition - generalMetrics.targetCentroid );
			double postDistanceToTargetCentroid = c3ga::norm( destinationPosition - generalMetrics.targetCentroid );
			double deltaToTargetCentroid = postDistanceToTargetCentroid - preDistanceToTargetCentroid;
			metrics->netDeltaToTargetCentroid += deltaToTargetCentroid;
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