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
	struct GeneralMetrics;

	Brain( void );
	virtual ~Brain( void );

	// Try to determine a good move that could be made by the given participant based upon the current state of the game and our internal cache.
	// Note that we do make changes to the given board, but we should always return it to its originally given state.
	bool FindGoodMoveForParticipant( int color, Board* board, Board::Move& move );

	// Here we examine every outcome of the game up to the given move count as if every turn was our own.
	void ExamineEveryOutcomeForBestMoveSequence( int color, Board* board, const GeneralMetrics& generalMetrics, Board::MoveList& moveList, int maxMoveCount, Cache*& cache, int sourceID );

	//=====================================================================================
	struct GeneralMetrics
	{
		c3ga::vectorE3GA generalMoveDir;
		c3ga::vectorE3GA targetCentroid;
		Board::Location* targetVertexLocation;
		Board::Location* sourceVertexLocation;
		Board::Location* stragglerLocation;
		Board::Location* leaderLocation;
	};

	//=====================================================================================
	class Cache
	{
	private:

		struct Metrics;

	public:

		Cache( Board::MoveList* moveList = nullptr );
		virtual ~Cache( void );

		void SetMoveList( Board::MoveList* moveList );
		int GetMoveListSize( void ) { return moveList.size(); }

		bool IsValid( Board* board );
		bool MakeNextMove( Board::Move& move );

		// Return true if the given cache is better than this cache.
		bool Compare( int color, Board* board, const GeneralMetrics& generalMetrics, Cache* cache );

		Metrics* GetMetrics( int color, Board* board, const GeneralMetrics& generalMetrics );

	private:

		bool RecursivelyValidateMoveList( Board* board, Board::MoveList::iterator& iter );

		Board::MoveList moveList;

		struct Metrics
		{
			double netProjectedSignedDistance;
			int targetZoneLandingCount;
			double totalDistanceToTargetCentroid;
		};

		Metrics* metrics;
	};

	void CalculateGeneralMetrics( int color, Board* board, GeneralMetrics& generalMetrics );

	typedef std::map< int, Cache* > CacheMap;
	CacheMap cacheMap;

	void FreeCacheMap( void );

	void ImproveMove( int color, Board* board, Board::Move& move, const GeneralMetrics& generalMetrics );
	void ImproveMoveRecursively( Board::Location* location, Board::Location* targetVertexLocation, Board::Location*& bestLocation );
};

// ChiCheBrain.h
