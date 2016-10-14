// ChiCheBrain.cpp

#include "ChiCheBrain.h"
#include <wx/thread.h>
#include <wx/progdlg.h>

using namespace ChiChe;

//=====================================================================================
Brain::Brain( void )
{
}

//=====================================================================================
/*virtual*/ Brain::~Brain( void )
{
	WipePlanList();
}

//=====================================================================================
void Brain::WipePlanList( void )
{
	while( planList.size() > 0 )
	{
		PlanList::iterator iter = planList.begin();
		Plan* plan = *iter;
		delete plan;
		planList.erase( iter );
	}
}

//=====================================================================================
bool Brain::FindGoodMoveForWhosTurnItIs( Board* board, Board::Move& move )
{
	int whosTurn = board->WhosTurn();
	int zoneTarget = board->ZoneTarget( whosTurn );

	Board::Location* targetVertexLocation = nullptr;
	for( Board::LocationMap::iterator iter = board->locationMap.begin(); iter != board->locationMap.end(); iter++ )
	{
		Board::Location* location = iter->second;
		if( location->GetZone() == zoneTarget && location->GetAdjacencyCount() == 2 )
		{
			targetVertexLocation = location;
			break;
		}
	}

	wxASSERT( targetVertexLocation );

	Plan* plan = new Plan();

	int depth = 2;
	Board::MoveList moveList;
	ExploreAllPossibleOutcomesToFindBestPlan( board, moveList, depth, plan, targetVertexLocation );

	planList.push_back( plan );

	Plan* bestPlan = nullptr;

	PlanList::iterator planIter = planList.begin();
	while( planIter != planList.end() )
	{
		plan = *planIter;
		PlanList::iterator nextPlanIter = planIter;
		nextPlanIter++;

		Board::MoveSequence moveSequence;
		if( plan->moveList.size() == 0 || !board->FindMoveSequence( *plan->moveList.begin(), moveSequence ) )
		{
			delete plan;
			planList.erase( planIter );
		}
		else
		{
			bestPlan = ChooseBetterPlan( bestPlan, plan, board, false );
		}

		planIter = nextPlanIter;
	}

	if( !bestPlan )
		return false;

	move = *bestPlan->moveList.begin();
	bestPlan->moveList.pop_front();
	return true;
}

//=====================================================================================
Brain::Plan* Brain::ChooseBetterPlan( Plan* planA, Plan* planB, Board* board, bool deleteWorsePlan )
{
	Plan* betterPlan = nullptr;

	while( true )
	{
		if( !planA )
		{
			betterPlan = planB;
			break;
		}

		if( !planB )
		{
			betterPlan = planA;
			break;
		}

		if( planA->moveList.size() == 0 )
		{
			betterPlan = planB;
			break;
		}

		if( planB->moveList.size() == 0 )
		{
			betterPlan = planA;
			break;
		}

		double eps = 1e-5;

		if( fabs( planA->totalDistanceToTarget - planB->totalDistanceToTarget ) >= eps )
		{
			if( planA->totalDistanceToTarget < planB->totalDistanceToTarget )
			{
				betterPlan = planA;
				break;
			}

			if( planB->totalDistanceToTarget < planA->totalDistanceToTarget )
			{
				betterPlan = planB;
				break;
			}
		}

		double moveDistanceA = planA->CalculateInitialMoveDistance( board );
		double moveDistanceB = planB->CalculateInitialMoveDistance( board );

		if( fabs( moveDistanceA - moveDistanceB ) >= eps )
		{
			if( moveDistanceA > moveDistanceB )
			{
				betterPlan = planA;
				break;
			}

			if( moveDistanceB > moveDistanceA )
			{
				betterPlan = planB;
				break;
			}
		}

		// Here we could randomly choose between either plan as we have
		// determined that they are equally matched.  Choose one arbitrarily for now.
		betterPlan = planA;
		break;
	}

	if( deleteWorsePlan )
	{
		if( betterPlan == planA )
			delete planB;
		else if( betterPlan == planB )
			delete planA;
	}

	return betterPlan;
}

//=====================================================================================
void Brain::ExploreAllPossibleOutcomesToFindBestPlan( Board* board, Board::MoveList& moveList, int depth, Plan*& plan, Board::Location* targetVertexLocation )
{
	int whosTurn = board->WhosTurn();
	int winningColor = board->DetermineWinner();
	int zoneTarget = board->ZoneTarget( whosTurn );

	if( moveList.size() == depth || winningColor == whosTurn )
	{
		double totalDistanceToTarget = board->CalculateTotalDistanceToLocation( whosTurn, targetVertexLocation );
		Plan* newPlan = new Plan( &moveList, totalDistanceToTarget );
		plan = ChooseBetterPlan( newPlan, plan, board, true );
	}
	else
	{
		Board::LocationList sourceLocationList;
		board->FindParticpantLocations( whosTurn, sourceLocationList );

		for( Board::LocationList::iterator srcIter = sourceLocationList.begin(); srcIter != sourceLocationList.end(); srcIter++ )
		{
			Board::Location* sourceLocation = *srcIter;

			bool noIntermediates = false;
			if( depth >= 3 )
				noIntermediates = true;

			Board::LocationList destinationLocationList;
			board->FindAllPossibleDestinations( sourceLocation, destinationLocationList, noIntermediates );

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

				ExploreAllPossibleOutcomesToFindBestPlan( board, moveList, depth, plan, targetVertexLocation );

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
Brain::Plan::Plan( Board::MoveList* moveList /*= nullptr*/, double totalDistanceToTarget /*= 0.0*/ )
{
	if( moveList )
		for( Board::MoveList::iterator iter = moveList->begin(); iter != moveList->end(); iter++ )
			this->moveList.push_back( *iter );
	
	this->totalDistanceToTarget = totalDistanceToTarget;
}

//=====================================================================================
/*virtual*/ Brain::Plan::~Plan( void )
{
}

//=====================================================================================
double Brain::Plan::CalculateInitialMoveDistance( Board* board )
{
	return board->CalculateMoveDistance( *moveList.begin() );
}

// ChiCheBrain.cpp