// ChiCheServer.h

namespace ChiChe
{
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
		public:

			Socket* socket;
			int color;
		};

		typedef std::list< Participant* > ParticipantList;

		Board* board;
		wxSocketServer* socket;
		ParticipantList participantList;
	};
}

// ChiCheServer.h