// ChiCheClient.h

#pragma once

#include "ChiCheBoard.h"
#include <wx/event.h>
#include <wx/glcanvas.h>

namespace ChiChe
{
	class Brain;
	
	//=====================================================================================
	class Client : public wxEvtHandler
	{
	public:

		enum Type
		{
			HUMAN,
			COMPUTER,
		};

		Client( Type type );
		~Client( void );

		bool Connect( const wxIPV4address& address );
		bool Run( void );
		bool Render( GLenum renderMode );
		bool Animate( double frameRate );
		bool ProcessHitList( unsigned int* hitBuffer, int hitBufferSize, int hitCount );

		int GetColor( void ) { return color; }
		Socket* GetSocket( void ) { return socket; }
		Board* GetBoard( void ) { return board; }

	private:

		void TellUserWhosTurnItIs( void );

		void OnBoardThinking( Board::Event& event );
		void UpdateThinkingStatus( const Socket::Packet& packet );

		Socket* socket;
		Board* board;
		Type type;
		int color;
		int selectedLocationID;
		bool movePacketSent;
		Brain* brain;
	};
}

// ChiCheClient.h