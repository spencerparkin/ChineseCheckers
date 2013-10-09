// ChiCheClient.h

namespace ChiChe
{
	//=====================================================================================
	class Client : public wxEvtHandler
	{
	public:

		enum PacketType
		{
			GAME_MOVE,
			BEGIN_COMPUTER_THINKING,
			UPDATE_COMPUTER_THINKING,
			END_COMPUTER_THINKING,
		};

		enum Type
		{
			HUMAN,
			COMPUTER_LEVEL_1,
			COMPUTER_LEVEL_2,
			COMPUTER_LEVEL_3,
		};

		Client( Type type );
		~Client( void );

		bool Connect( const wxIPV4address& address );
		bool Run( void );
		bool Render( GLenum renderMode );
		bool Animate( double frameRate );
		bool ProcessHitList( unsigned int* hitBuffer, int hitBufferSize, int hitCount );

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
		Board::MoveMemory moveMemory;
	};
}

// ChiCheClient.h