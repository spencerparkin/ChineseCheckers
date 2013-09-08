// ChiCheServer.h

namespace ChiChe
{
	//=====================================================================================
	class Server
	{
	public:

		enum PacketType
		{
			ASSIGN_COLOR,
			PARTICIPANTS,
			GAME_MOVE,
			GAME_STATE,
		};

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

		private:
			Socket* socket;
			int color;
		};

		typedef std::list< Participant* > ParticipantList;

		int AssignColorToNewParticipant( void );
		bool ColorAlreadyAssigned( int color );

		bool ServiceClient( Participant* participant );

		void BroadcastPacket( Socket::Packet& outPacket );

		Board* board;
		wxSocketServer* socketServer;
		ParticipantList participantList;
	};
}

// ChiCheServer.h