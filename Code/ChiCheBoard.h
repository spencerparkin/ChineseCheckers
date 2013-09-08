// ChiCheBoard.h

namespace ChiChe
{
	//=====================================================================================
	// This represents the game board, which is the entire state of the game, including
	// who's turn it is, and where all the player pieces are located.
	class Board
	{
	public:

		enum
		{
			ADJACENCIES = 6,
			PIECES_PER_ZONE = 10,
			LAYOUT_SIZE = 17,
			LOCATION_EDGE_LENGTH = 5,
		};

		// Many things depend on the order of these enums!
		enum
		{
			NONE,
			RED,
			GREEN,
			BLUE,
			YELLOW,
			MAGENTA,
			CYAN,
		};

		static int layoutMap[ LAYOUT_SIZE ][ LAYOUT_SIZE ];

		class Location;
		class Piece;

		typedef std::list< int > MoveSequence;
		typedef std::map< int, Location* > LocationMap;
		typedef std::list< Piece* > PieceList;
		typedef std::map< int, bool > VisitationMap;

		// Construct a game board with the given participants.
		Board( int participants, bool animate );
		~Board( void );

		// Which zone are pieces of the given color trying to get to?
		static int ZoneTarget( int color );

		// Grab a game move, if any, found in the given packet.
		static bool UnpackMove( const Socket::Packet& packet, wxInt32& sourceID, wxInt32& destinationID );

		// Is the given color particpating in the game?
		bool IsParticipant( int color );

		// Is it the given participant's turn?
		bool IsParticipantsTurn( int color );

		// Get the participants bit field.
		int GetParticipants( void );

		// Return the occupant at the given location.
		int OccupantAtLocation( int locationID );

		// Return the winning piece color, if any.
		int DetermineWinner( void );

		// Render the board for display or selection.
		void Render( GLenum renderMode );

		// Advance the position of each board piece by one frame's worth of animation.
		void Animate( double frameRate );

		// Return the location ID of the location selected by the user, if any.
		int FindSelectedLocation( unsigned int* hitBuffer, int hitBufferSize, int hitCount );

		// If a valid sequence of moves exists from the given source location to the given destination location, return it as a move sequence.
		// Note that this also takes into account who's turn it is.
		bool FindMoveSequence( int sourceID, int destinationID, MoveSequence& moveSequence );

		// Verify the validity of the given move sequence.  Note that this also takes into account who's turn it is.
		bool IsMoveSequenceValid( const MoveSequence& moveSequence );

		// Change the state of the game board by the given move sequence.  This will also change who's turn it is.
		bool ApplyMoveSequence( const MoveSequence& moveSequence );

		// Package the state of the entire game into a packet.
		bool GetGameState( Socket::Packet& outPacket );

		// Internalize the entire state of the game that is in the given packet.
		bool SetGameState( const Socket::Packet& inPacket );

		//=====================================================================================
		// Each of these represents a location on the game board.
		class Location
		{
		public:

			Location( int occupant, int zone, int locationID );
			~Location( void );

			void Render( GLenum renderMode );

			void CalcPositionOfAdjacencies( const c3ga::vectorE3GA& position, VisitationMap& visitationMap );

			void SetAdjacency( int index, Location* location );
			Location* GetAdjacency( int index );

			int GetZone( void );

			void SetOccupant( int occupant );
			int GetOccupant( void );

		private:

			c3ga::vectorE3GA position;
			Location* adjacency[ ADJACENCIES ];
			int occupant;
			int zone;
			int locationID;
			Piece* piece;
		};

		//=====================================================================================
		// A board piece is used to show which board locations are occupied by which players.
		// These animate and lag behind the true location of each piece.
		class Piece
		{
		public:

			Piece( int color );
			~Piece( void );

			void Render( void );
			void Animate( double frameRate );

		private:

			int color;
			MoveSequence moveSequence;
			double pivotAngle;
		};

	private:

		void CreateGraph( bool animate );
		void DestroyGraph( void );

		// These are internal, because only the application of a move causes us to change who's turn it is.
		void NextTurn( void );
		void IncrementTurn( void );

		int participants;
		int whosTurn;
		LocationMap locationMap;
		PieceList pieceList;
	};
}

// ChiCheBoard.h