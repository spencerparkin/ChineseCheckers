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

	// Note that we do make changes to the given board here, but we should always return it to its originally given state.
	bool FindGoodMoveForParticipant( int color, Board* board, Board::Move& move );

	//...
};

// ChiCheBrain.h
