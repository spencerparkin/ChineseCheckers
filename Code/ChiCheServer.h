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
			DROPPED_CLIENT,
			SCORE_BONUS,
			BEGIN_COMPUTER_THINKING,
			UPDATE_COMPUTER_THINKING,
			END_COMPUTER_THINKING,
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