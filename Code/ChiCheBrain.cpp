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
}

//=====================================================================================
bool Brain::FindGoodMoveForWhosTurnItIs( Board* board, Board::Move& move )
{
	int depth = 2;

	Board::MoveList moveList;
	ExplorePossibleOutcomes( board, moveList, depth );

	return false;
}

//=====================================================================================
void Brain::ExplorePossibleOutcomes( Board* board, Board::MoveList& moveList, int depth )
{
	int whosTurn = board->WhosTurn();
	int winningColor = board->DetermineWinner();
	int zoneTarget = board->ZoneTarget( whosTurn );

	if( moveList.size() == depth || winningColor == whosTurn )
	{
		//...
	}
	else
	{
		Board::LocationList sourceLocationList;
		board->FindParticpantLocations( whosTurn, sourceLocationList );

		for( Board::LocationList::iterator srcIter = sourceLocationList.begin(); srcIter != sourceLocationList.end(); srcIter++ )
		{
			Board::Location* sourceLocation = *srcIter;

			Board::LocationList destinationLocationList;
			board->FindAllPossibleDestinations( sourceLocation, destinationLocationList );

			for( Board::LocationList::iterator dstIter = destinationLocationList.begin(); dstIter != destinationLocationList.end(); dstIter++ )
			{
				Board::Location* destinationLocation = *dstIter;
				int zone = destinationLocation->GetZone();
				if( !( zone == Board::NONE || zone == whosTurn || zone == zoneTarget ) )
					continue;

				Board::Move move;
				move.sourceID = sourceLocation->GetLocationID();
				move.destinationID = destinationLocation->GetLocationID();

				bool applied = board->ApplyMoveInternally( move );
				wxASSERT( applied );

				moveList.push_back( move );

				ExplorePossibleOutcomes( board, moveList, depth );

				moveList.pop_back();

				Board::Move invMove;
				move.Inverse( invMove );

				applied = board->ApplyMoveInternally( invMove );
				wxASSERT( applied );
			}
		}
	}
}

// ChiCheBrain.cpp