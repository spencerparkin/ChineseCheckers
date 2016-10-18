// ChiCheBrain.h

#pragma once

#include "ChiCheBoard.h"

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

	// Note that we may make changes to the given board here, but we should always return it to its originally given state.
	bool FindGoodMoveForWhosTurnItIs( Board* board, Board::Move& move );

private:

	class Plan
	{
	public:

		Plan( Board::MoveList* moveList = nullptr, double totalDistanceToTarget = 0.0 );
		virtual ~Plan( void );

		Board::MoveList moveList;

		// If our plan is completely executed, it will bring us to within this distance of the target.
		double totalDistanceToTarget;

		double CalculateInitialMoveDistance( Board* board );
	};

	typedef std::list< Plan* > PlanList;
	PlanList planList;

	// Here we approximate all possible outcomes to a given depth by pretending it's always our turn.
	void ExploreAllPossibleOutcomesToFindBestPlan( Board* board, Board::MoveList& moveList, int depth, Plan*& plan, Board::Location* targetVertexLocation );

	// The given board does not necessarily reflect either plan applied; we just need it for some calculations.
	Plan* ChooseBetterPlan( Plan* planA, Plan* planB, Board* board, bool deleteWorsePlan );

	void WipePlanList( void );
};

// ChiCheBrain.h
