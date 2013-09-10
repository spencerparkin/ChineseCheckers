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
}

//=====================================================================================
Client::~Client( void )
{
	delete socket;
	delete board;
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
				break;
			}
		}
	}

	if( type == COMPUTER && board )
	{
		// Wait until it's our turn.
		if( !board->IsParticipantsTurn( color ) )
			movePacketSent = false;
		else if( !movePacketSent && !board->AnyPieceInMotion() )
		{
			// Okay, it's time to make our move.
			int sourceID, destinationID;
			if( !board->FindBestMoveForParticipant( color, sourceID, destinationID ) )
				return false;
			else
			{
				Socket::Packet outPacket;
				Board::PackMove( outPacket, sourceID, destinationID, GAME_MOVE );
				socket->WritePacket( outPacket );
				movePacketSent = true;
			}
		}
	}

	return true;
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