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

	// Note that we may make changes to the given board here, but we should always return it to its originally given state.
	bool FindGoodMoveForWhosTurnItIs( Board* board, Board::Move& move );

private:

	// Here we approximate all possible outcomes to a given depth by pretending it's always our turn.
	void ExplorePossibleOutcomes( Board* board, Board::MoveList& moveList, int depth );
};

// ChiCheBrain.h
