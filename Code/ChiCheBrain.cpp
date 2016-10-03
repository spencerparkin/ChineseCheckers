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

	Board::MoveList moveList;
	ExamineEveryOutcomeForBestMoveSequence( color, board, moveList, 3, cache );

	cache->MakeNextMove( move );
	return true;
}

//=====================================================================================
void Brain::ExamineEveryOutcomeForBestMoveSequence( int color, Board* board, Board::MoveList& moveList, int maxMoveCount, Cache*& cache )
{
	int winningColor = board->DetermineWinner();
	
	if( moveList.size() == maxMoveCount || winningColor == color )
	{
		Cache* newCache = new Cache( &moveList );

		if( cache->Compare( color, board, newCache ) )
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

				ExamineEveryOutcomeForBestMoveSequence( color, board, moveList, maxMoveCount, cache );

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
	SetMoveList( moveList );
}

//=====================================================================================
/*virtual*/ Brain::Cache::~Cache( void )
{
}

//=====================================================================================
void Brain::Cache::SetMoveList( Board::MoveList* moveList )
{
	moveListInPlay.clear();

	if( moveList )
	{
		Board::MoveList::iterator iter = moveList->begin();
		while( iter != moveList->end() )
		{
			moveListInPlay.push_back( *iter );
			iter++;
		}
	}
}

//=====================================================================================
bool Brain::Cache::IsValid( Board* board )
{
	if( moveListInPlay.size() == 0 )
		return false;

	Board::MoveList::iterator iter = moveListInPlay.begin();
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
	if( moveListInPlay.size() == 0 )
		return false;

	move = *moveListInPlay.begin();
	moveListInPlay.pop_front();

	return true;
}

//=====================================================================================
bool Brain::Cache::Compare( int color, Board* board, Cache* cache )
{
	// In order of significance...
	// 1) how many full to we make the target zone become?
	// 2) smallest distance computed between target vertex and furthest-behind marble,
	// 3) net distance moved by the entire move sequence (use integer measure: -2, -1, 0, 1, 2 per jump.
	// To enforce significance, shift by 10s.

	return 0;
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