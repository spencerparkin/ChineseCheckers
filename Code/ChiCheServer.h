// ChiCheServer.h

namespace ChiChe
{
	//=====================================================================================
	class Server
	{
	public:

		Server( int participants );
		~Server( void );

		bool Initialize( unsigned int port );
		bool Finalize( void );
		bool Run( void );

	private:

		//=====================================================================================
		class Participant
		{
		public:

			wxSocketBase* socket;
			int color;
		};

		Board* board;
		wxSocketServer* socket;
	};
}

// ChiCheServer.h