// ChiCheServer.cpp

#include "ChiCheServer.h"
#include "ChiCheClient.h"
#include "ChiCheBoard.h"

using namespace ChiChe;

//=====================================================================================
Server::Server( int participants )
{
	socketServer = 0;
	board = new Board( participants, false );
	gameOver = false;
}

//=====================================================================================
Server::~Server( void )
{
	delete board;
	delete socketServer;
}

//=====================================================================================
bool Server::Initialize( unsigned short port )
{
	wxIPV4address address;
	address.LocalHost();
	address.Service( port );

	socketServer = new wxSocketServer( address, wxSOCKET_NOWAIT );

	if( !socketServer->Initialize() )
		return false;

	return true;
}

//=====================================================================================
bool Server::Finalize( void )
{
	while( participantList.size() > 0 )
	{
		ParticipantList::iterator iter = participantList.begin();
		Participant* participant = *iter;
		delete participant;
		participantList.erase( iter );
	}

	socketServer->Shutdown();
	delete socketServer;
	socketServer = 0;

	return true;
}

//=====================================================================================
bool Server::Run( void )
{
	// Listen for incoming client connections unless the game is over.
	if( !gameOver )
	{
		wxSocketBase* connectedSocket = socketServer->Accept( false );
		if( connectedSocket )
		{
			int color = AssignColorToNewParticipant();
			Participant* participant = new Participant( connectedSocket, color );
			participantList.push_back( participant );

			Socket::Packet outPacket;

			outPacket.SetType( Socket::Packet::ASSIGN_COLOR );
			outPacket.SetData( ( wxInt8* )&color );
			outPacket.SetSize( sizeof( int ) );
			outPacket.OwnsMemory( false );
			participant->socket->WritePacket( outPacket );

			int participants = board->GetParticipants();
			outPacket.Reset();
			outPacket.SetType( Socket::Packet::PARTICIPANTS );
			outPacket.SetData( ( wxInt8* )&participants );
			outPacket.SetSize( sizeof( int ) );
			outPacket.OwnsMemory( false );
			participant->socket->WritePacket( outPacket );

			board->GetGameState( outPacket );
			participant->socket->WritePacket( outPacket );
		}
	}

	// Continually service all connected clients.
	ParticipantList::iterator iter = participantList.begin();
	while( iter != participantList.end() )
	{
		Participant* participant = *iter;
		ParticipantList::iterator eraseIter = iter;
		iter++;
		if( !ServiceClient( participant ) )
		{
			wxInt32 color = participant->Color();
			delete participant;
			participantList.erase( eraseIter );

			Socket::Packet outPacket;
			outPacket.SetType( Socket::Packet::DROPPED_CLIENT );
			outPacket.SetData( ( wxInt8* )&color );
			outPacket.SetSize( sizeof( wxInt32 ) );
			outPacket.OwnsMemory( false );
			BroadcastPacket( outPacket );
		}
	}

	return true;
}

//=====================================================================================
bool Server::ServiceClient( Participant* participant )
{
	if( !participant->socket->Advance() )
		return false;

	Socket::Packet inPacket;
	if( participant->socket->ReadPacket( inPacket ) )
	{
		switch( inPacket.GetType() )
		{
			case Socket::Packet::GAME_MOVE:
			{
				// Don't let clients make moves if someone has one the game.
				if( Board::NONE != board->DetermineWinner() )
					break;

				// Grab the move's source and destination locations.
				wxInt32 sourceID, destinationID;
				if( !board->UnpackMove( inPacket, sourceID, destinationID ) )
					break;
				
				// A participant can only move the color to which it is assigned and can't make a move if a spectator.
				if( participant->color == Board::NONE || board->OccupantAtLocation( sourceID ) != participant->color )
					break;

				// Make sure the move is valid.  Formulate from it a move sequence.
				Board::MoveSequence moveSequence;
				if( !board->FindMoveSequence( sourceID, destinationID, moveSequence ) )
					break;
				
				// Perform the move sequence on the master board.
				if( !board->ApplyMoveSequence( moveSequence, false ) )
					break;
				
				// Now go have all the clients make the same move to keep all boards in sync.
				Socket::Packet outPacket;
				Board::PackMove( outPacket, sourceID, destinationID );
				BroadcastPacket( outPacket );

				// If the game is over, we're done.
				if( board->DetermineWinner() != Board::NONE )
					gameOver = true;

				break;
			}
			case Socket::Packet::SCORE_BONUS:
			{
				wxInt32 participant;
				wxInt64 scoreBonus;
				Board::UnpackScoreBonus( inPacket, participant, scoreBonus );

				if( !board->ApplyScoreBonus( participant, scoreBonus ) )
					break;

				Socket::Packet outPacket;
				Board::PackScoreBonus( outPacket, participant, scoreBonus );
				BroadcastPacket( outPacket );

				break;
			}
			case Socket::Packet::BEGIN_COMPUTER_THINKING:
			case Socket::Packet::UPDATE_COMPUTER_THINKING:
			case Socket::Packet::END_COMPUTER_THINKING:
			{
				wxInt32 type;
				if( inPacket.GetType() == Socket::Packet::BEGIN_COMPUTER_THINKING )
					type = Socket::Packet::BEGIN_COMPUTER_THINKING;
				else if( inPacket.GetType() == Socket::Packet::UPDATE_COMPUTER_THINKING )
					type = Socket::Packet::UPDATE_COMPUTER_THINKING;
				else if( inPacket.GetType() == Socket::Packet::END_COMPUTER_THINKING )
					type = Socket::Packet::END_COMPUTER_THINKING;

				// Broad-cast the messate to all but the sender, because
				// the sender is too busy to receive the message.
				Socket::Packet outPacket;
				outPacket.SetType( type );
				outPacket.SetData( inPacket.GetData() );
				outPacket.SetSize( inPacket.GetSize() );
				outPacket.OwnsMemory( false );
				BroadcastPacket( outPacket, participant );
				break;
			}
		}
	}

	return true;
}

//=====================================================================================
void Server::BroadcastPacket( Socket::Packet& outPacket, Participant* excludedParticipant /*= 0*/ )
{
	for( ParticipantList::iterator iter = participantList.begin(); iter != participantList.end(); iter++ )
	{
		Participant* participant = *iter;
		if( participant != excludedParticipant )
			participant->socket->WritePacket( outPacket );
	}
}

//=====================================================================================
int Server::AssignColorToNewParticipant( void )
{
	int color = Board::RED;
	while( ColorAlreadyAssigned( color ) || !board->IsParticipant( color ) )
		if( ++color > Board::CYAN )
			return Board::NONE;		// These types of participants can only be spectators.
	return color;
}

//=====================================================================================
bool Server::ColorAlreadyAssigned( int color )
{
	for( ParticipantList::iterator iter = participantList.begin(); iter != participantList.end(); iter++ )
	{
		Participant* participant = *iter;
		if( participant->color == color )
			return true;
	}
	return false;
}

//=====================================================================================
Server::Participant::Participant( wxSocketBase* connectedSocket, int color )
{
	socket = new Socket( connectedSocket );

	this->color = color;
}

//=====================================================================================
Server::Participant::~Participant( void )
{
	delete socket;
}

//=====================================================================================
int Server::Participant::Color( void )
{
	return color;
}

// ChiCheServer.cpp