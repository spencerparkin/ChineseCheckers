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
		};

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

		class Location;
		class Piece;

		typedef std::list< int > MoveSequence;
		typedef std::map< int, Location* > LocationMap;
		typedef std::list< Piece* > PieceList;

		// Construct a game board with the given participants.
		Board( int participants );
		~Board( void );

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

		//=====================================================================================
		// Each of these represents a location on the game board.
		class Location
		{
		public:

			Location( const c3ga::vectorE3GA& position, int occupant, int zone, int locationID );
			~Location( void );

			void Render( GLenum renderMode );

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

		int particpants;
		int whosTurn;
		LocationMap locationMap;
		PieceList pieceList;
	};
}

// ChiCheBoard.h