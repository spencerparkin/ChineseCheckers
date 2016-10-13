// ChiCheClient.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Client::Client( Type type )
{
	this->type = type;
	socket = 0;
	board = 0;
	color = Board::NONE;
	selectedLocationID = -1;
	movePacketSent = false;
	brain = nullptr;
	if( type == COMPUTER )
		brain = new Brain();
}

//=====================================================================================
Client::~Client( void )
{
	delete socket;
	delete board;
	delete brain;
}

//=====================================================================================
bool Client::Connect( const wxIPV4address& address )
{
	wxSocketClient* socketClient = new wxSocketClient( wxSOCKET_NOWAIT );
	socket = new Socket( socketClient );

	// Try once before throwing up the progress dialog.
	socketClient->Connect( address, false );
	if( socketClient->WaitOnConnect( 1, 0 ) && socketClient->IsConnected() )
		return true;
	
	wxString hostName = address.Hostname();
	int tryCount = 0;
	int maxTries = 10;
	wxString progressMessage = wxT( "Connecting..." );
	wxGenericProgressDialog progressDialog( wxT( "Connecting to host: " ) + hostName, progressMessage, maxTries, wxGetApp().GetFrame(), wxPD_APP_MODAL | wxPD_CAN_ABORT );
	while( tryCount < maxTries )
	{
		socketClient->Connect( address, false );
		if( socketClient->WaitOnConnect( 1, 0 ) && socketClient->IsConnected() )
			return true;

		tryCount++;
		if( tryCount == maxTries )
			progressMessage = wxT( "Connection attempt failed!" );

		bool cancel = !progressDialog.Update( tryCount, progressMessage );
		if( cancel )
			break;
	}

	return false;
}

//=====================================================================================
bool Client::Run( void )
{
	if( !socket->Advance() )
		return false;

	Socket::Packet inPacket;
	if( socket->ReadPacket( inPacket ) )
	{
		switch( inPacket.GetType() )
		{
			case Server::GAME_STATE:
			{
				if( !board || !board->SetGameState( inPacket ) )
					return false;
				TellUserWhosTurnItIs();
				break;
			}
			case Server::GAME_MOVE:
			{
				wxInt32 sourceID, destinationID;
				if( board->UnpackMove( inPacket, sourceID, destinationID ) )
				{
					Board::MoveSequence moveSequence;
					if( !board->FindMoveSequence( sourceID, destinationID, moveSequence ) )
						return false;
					if( !board->ApplyMoveSequence( moveSequence ) )
						return false;
					TellUserWhosTurnItIs();
					int winner = board->DetermineWinner();
					if( winner != Board::NONE )
					{
						wxString winnerText;
						Board::ParticipantText( winner, winnerText );
						wxMessageBox( wxT( "Player " ) + winnerText + wxT( " wins!" ), wxT( "The game is over!" ), wxOK | wxCENTRE, wxGetApp().GetFrame() );
					}
				}
				break;
			}
			case Server::SCORE_BONUS:
			{
				wxInt32 participant;
				wxInt64 scoreBonus;
				Board::UnpackScoreBonus( inPacket, participant, scoreBonus );

				board->ApplyScoreBonus( participant, scoreBonus );

				// TODO: Play "Bonus!" wave?

				break;
			}
			case Server::ASSIGN_COLOR:
			{
				wxInt32 data = *( wxInt32* )inPacket.GetData();
				if( inPacket.ByteSwap() )
					data = wxINT32_SWAP_ALWAYS( data );
				color = data;
				break;
			}
			case Server::PARTICIPANTS:
			{
				if( board )
					return false;
				wxInt32 data = *( wxInt32* )inPacket.GetData();
				if( inPacket.ByteSwap() )
					data = wxINT32_SWAP_ALWAYS( data );
				int participants = data;
				board = new Board( participants, true );
				board->SetEventHandler( this );
				Bind( Board::BEGIN_BOARD_THINKING, &Client::OnBoardThinking, this );
				Bind( Board::UPDATE_BOARD_THINKING, &Client::OnBoardThinking, this );
				Bind( Board::END_BOARD_THINKING, &Client::OnBoardThinking, this );
				break;
			}
			case Server::DROPPED_CLIENT:
			{
				wxInt32 color = *( wxInt32* )inPacket.GetData();
				if( inPacket.ByteSwap() )
					color = wxINT32_SWAP_ALWAYS( color );
				wxString textColor;
				Board::ParticipantText( color, textColor );
				wxString message = wxT( "The player for color " ) + textColor + wxT( " has been dropped by the server, probably due to connection failure.  The game will halt at this player's turn until another client joins the game." );
				wxMessageBox( message, wxT( "Dropped Client" ), wxOK | wxCENTRE, wxGetApp().GetFrame() );
				break;
			}
			case Server::BEGIN_COMPUTER_THINKING:
			case Server::UPDATE_COMPUTER_THINKING:
			case Server::END_COMPUTER_THINKING:
			{
				UpdateThinkingStatus( inPacket );
				break;
			}
		}
	}

	// If we're an computer participant, go run the AI logic.
	while( type == COMPUTER )
	{
		// There's nothing for us to do until the board is created, and we have to have a brain.
		if( !board || !brain )
			break;

		// Wait until it's our turn.
		if( !board->IsParticipantsTurn( color ) )
		{
			movePacketSent = false;
			break;
		}

		// Wait for our turn to take affect if we've already taken it.
		if( movePacketSent )
			break;

		// Wait for all animations to settle before taking our turn.
		if( board->AnyPieceInMotion() )
			break;
			
		// If someone has just won the game, don't submit any moves; the game is over.
		if( board->DetermineWinner() != Board::NONE )
			break;
		
		// Okay, it's time to make our move.
		Board::Move move;
		if( !brain->FindGoodMoveForWhosTurnItIs( board, move ) )
		{
			wxString textColor;
			Board::ParticipantText( color, textColor );
			wxString message = wxT( "The computer player for color " ) + textColor + wxT( " is stumped and sadly can't continue the game." );
			wxMessageBox( message, wxT( "The Computer Is Stupid" ), wxOK | wxCENTRE, wxGetApp().GetFrame() );
			return false;
		}
		
		// Submit our move!
		Socket::Packet outPacket;
		Board::PackMove( outPacket, move.sourceID, move.destinationID, GAME_MOVE );
		socket->WritePacket( outPacket );
		movePacketSent = true;
		break;
	}

	return true;
}

//=====================================================================================
void Client::OnBoardThinking( Board::Event& event )
{
	Socket::Packet outPacket;

	if( event.GetEventType() == Board::BEGIN_BOARD_THINKING )
		outPacket.SetType( BEGIN_COMPUTER_THINKING );
	else if( event.GetEventType() == Board::UPDATE_BOARD_THINKING )
		outPacket.SetType( UPDATE_COMPUTER_THINKING );
	else if( event.GetEventType() == Board::END_BOARD_THINKING )
		outPacket.SetType( END_COMPUTER_THINKING );

	wxString message = event.GetMessage();
	size_t size = message.Length() + 1;
	char* buffer = new char[ size ];
	//strcpy_s( buffer, size, ( const char* )message.c_str() );
	strcpy( buffer, ( const char* )message.c_str() );
	outPacket.SetData( ( wxInt8* )buffer );
	outPacket.SetSize( size );
	outPacket.OwnsMemory( true );

	socket->WritePacket( outPacket );
	socket->Advance();

	UpdateThinkingStatus( outPacket );
}

//=====================================================================================
void Client::UpdateThinkingStatus( const Socket::Packet& packet )
{
	Frame* frame = wxGetApp().GetFrame();
	wxStatusBar* statusBar = frame->GetStatusBar();
	if( !statusBar )
		return;

	wxString message = wxString( ( const char* )packet.GetData() );
	switch( packet.GetType() )
	{
		case Client::BEGIN_COMPUTER_THINKING:
		case Server::BEGIN_COMPUTER_THINKING:
		{
			statusBar->PushStatusText( message );
			break;
		}
		case Client::UPDATE_COMPUTER_THINKING:
		case Server::UPDATE_COMPUTER_THINKING:
		{
			statusBar->SetStatusText( message );
			break;
		}
		case Client::END_COMPUTER_THINKING:
		case Server::END_COMPUTER_THINKING:
		{
			statusBar->PopStatusText();
			break;
		}
	}

	statusBar->Refresh();
}

//=====================================================================================
bool Client::ProcessHitList( unsigned int* hitBuffer, int hitBufferSize, int hitCount )
{
	if( !board )
		return false;

	int locationID = board->FindSelectedLocation( hitBuffer, hitBufferSize, hitCount );
	
	if( selectedLocationID >= 0 && locationID >= 0 && selectedLocationID != locationID )
	{
		if( type == HUMAN )
		{
			Board::MoveSequence moveSequence;
			if( board->FindMoveSequence( selectedLocationID, locationID, moveSequence ) )
			{
				Socket::Packet outPacket;
				Board::PackMove( outPacket, selectedLocationID, locationID, GAME_MOVE );
				socket->WritePacket( outPacket );
			}
		}
	}

	selectedLocationID = locationID;

	return true;
}

//=====================================================================================
void Client::TellUserWhosTurnItIs( void )
{
	if( board )
	{
		wxString statusText, textColor;

		int winner = board->DetermineWinner();
		if( winner != Board::NONE )
		{
			Board::ParticipantText( winner, textColor );
			statusText += wxT( "Player " ) + textColor + wxT( " wins!" );
		}
		else
		{
			int whosTurn = board->WhosTurn();
			Board::ParticipantText( whosTurn, textColor );

			statusText = wxT( "It's " ) + textColor + wxT( "'s turn." );
			bool myTurn = board->IsParticipantsTurn( color );
			if( myTurn )
				statusText += wxT( "  (It's your turn!  Make your move!)" );
			else
				statusText += wxT( "  (It's not your turn yet!)" );
		}

		wxStatusBar* statusBar = wxGetApp().GetFrame()->GetStatusBar();
		statusBar->SetStatusText( statusText );
	}
}

//=====================================================================================
bool Client::Render( GLenum renderMode )
{
	if( board )
		board->Render( renderMode, selectedLocationID );

	return true;
}

//=====================================================================================
bool Client::Animate( double frameRate )
{
	if( board )
		board->Animate( frameRate );

	return true;
}

// ChiCheClient.cpp
