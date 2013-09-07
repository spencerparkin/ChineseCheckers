// ChiCheClient.h

namespace ChiChe
{
	//=====================================================================================
	class Client
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

	private:

		Socket* socket;
		Board* board;
		Type type;
		int color;
	};
}

// ChiCheClient.h