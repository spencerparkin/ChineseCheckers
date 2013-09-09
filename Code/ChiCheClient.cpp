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
				break;
			}
			case Server::GAME_MOVE:
			{
				wxInt32 sourceID, destinationID;
				if( board->UnpackMove( inPacket, sourceID, destinationID ) )
				{
					Board::MoveSequence moveSequence;
					if( board->FindMoveSequence( sourceID, destinationID, moveSequence ) )
						board->ApplyMoveSequence( moveSequence );
					else
						return false;
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

	return true;
}

//=====================================================================================
bool Client::Render( GLenum renderMode )
{
	if( board )
		board->Render( renderMode );

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