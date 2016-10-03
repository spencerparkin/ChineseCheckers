// ChiCheBrain.h

#pragma once

namespace ChiChe
{
	class Brain;
}

//=====================================================================================
class ChiChe::Brain
{
public:

	class Cache;

	Brain( void );
	virtual ~Brain( void );

	// Try to determine a good move that could be made by the given participant based upon the current state of the game and our internal cache.
	// Note that we do make changes to the given board, but we should always return it to its originally given state.
	bool FindGoodMoveForParticipant( int color, Board* board, Board::Move& move );

	// Here we examine every outcome of the game up to the given move count as if every turn was our own.
	void ExamineEveryOutcomeForBestMoveSequence( int color, Board* board, Board::MoveList& moveList, int maxMoveCount, Cache*& cache );

	//=====================================================================================
	class Cache
	{
	public:

		Cache( Board::MoveList* moveList = nullptr );
		virtual ~Cache( void );

		void SetMoveList( Board::MoveList* moveList );

		bool IsValid( Board* board );
		bool MakeNextMove( Board::Move& move );

		// Return true if the given cache is better than this cache.
		bool Compare( int color, Board* board, Cache* cache );

	private:

		bool RecursivelyValidateMoveList( Board* board, Board::MoveList::iterator& iter );

		Board::MoveList moveListInPlay;
	};

	typedef std::map< int, Cache* > CacheMap;
	CacheMap cacheMap;

	void FreeCacheMap( void );
};

// ChiCheBrain.h
