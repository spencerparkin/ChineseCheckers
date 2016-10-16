// ChiCheServer.h

#pragma once

#include "ChiCheSocket.h"
#include <list>

namespace ChiChe
{
	class Socket;
	class Board;

	//=====================================================================================
	class Server
	{
	public:

		Server( int participants );
		~Server( void );

		bool Initialize( unsigned short port );
		bool Finalize( void );
		bool Run( void );

	private:

		//=====================================================================================
		class Participant
		{
			friend class Server;

		public:
			Participant( wxSocketBase* connectedSocket, int color );
			~Participant( void );

			int Color( void );

		private:
			Socket* socket;
			int color;
		};

		typedef std::list< Participant* > ParticipantList;

		int AssignColorToNewParticipant( void );
		bool ColorAlreadyAssigned( int color );

		bool ServiceClient( Participant* participant );

		void BroadcastPacket( Socket::Packet& outPacket, Participant* excludedParticipant = 0 );

		Board* board;
		wxSocketServer* socketServer;
		ParticipantList participantList;
	};
}

// ChiCheServer.h