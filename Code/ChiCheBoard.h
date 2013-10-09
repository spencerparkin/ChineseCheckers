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

		struct Move
		{
			int sourceID;
			int destinationID;
		};

		typedef std::list< Move > MoveList;		// This is what one might also call a move sequence.  It is a sequence of moves made over one or more turns.
		typedef std::list< int > MoveSequence;	// Despite the name, this is just a sequence of moves that can be made in a single turn.
		typedef std::map< int, Location* > LocationMap;
		typedef std::list< Location* > LocationList;
		typedef std::list< Piece* > PieceList;
		typedef std::map< int, bool > VisitationMap;
		typedef std::map< int, void* > DestinationMap;
		typedef std::map< int, DestinationMap* > SourceMap;

		// This structure is used by the computer player logic, and
		// the same instance of it is expected to be passed to that
		// logic each turn it is given a chance to play.
		struct MoveMemory
		{
			MoveList moveListInPlay;
		};

		// Construct a game board with the given participants.
		Board( int participants, bool animate );
		~Board( void );

		// Which zone are pieces of the given color trying to get to?
		static int ZoneTarget( int color );

		// Return the color of the given zone/occupant/etc.
		static void RenderColor( int color, c3ga::vectorE3GA& renderColor );

		// Return the color of the given zone/occupant/etc. as a string.
		static void ParticipantText( int color, wxString& textColor );

		// Grab a game move, if any, found in the given packet.
		static bool UnpackMove( const Socket::Packet& inPacket, wxInt32& sourceID, wxInt32& destinationID );

		// Put a game move into the given packet.
		static bool PackMove( Socket::Packet& outPacket, wxInt32 sourceID, wxInt32 destinationID, int packetType );

		// Is the given color particpating in the game?
		bool IsParticipant( int color );

		// Is it the given participant's turn?
		bool IsParticipantsTurn( int color );

		// Whos turn is it?
		int WhosTurn( void );

		// Get the participants bit field.
		int GetParticipants( void );

		// Return the occupant at the given location.
		int OccupantAtLocation( int locationID );

		// Return the spacial position of the given board location.
		bool PositionAtLocation( int locationID, c3ga::vectorE3GA& position );

		// Return the winning piece color, if any.
		int DetermineWinner( void );

		// Render the board for display or selection.
		void Render( GLenum renderMode, int highlightLocationID );

		// Advance the position of each board piece by one frame's worth of animation.
		void Animate( double frameRate );

		// Return the location ID of the location selected by the user, if any.
		int FindSelectedLocation( unsigned int* hitBuffer, int hitBufferSize, int hitCount );

		// If a valid sequence of moves exists from the given source location to the given destination location, return it as a move sequence.
		// Note that this also takes into account who's turn it is.
		bool FindMoveSequence( int sourceID, int destinationID, MoveSequence& moveSequence );
		bool FindMoveSequence( const Move& move, MoveSequence& moveSequence );

		// Change the state of the game board by the given move sequence.  This will also change who's turn it is.
		bool ApplyMoveSequence( const MoveSequence& moveSequence );

		// Package the state of the entire game into a packet.
		bool GetGameState( Socket::Packet& outPacket );

		// Internalize the entire state of the game that is in the given packet.
		bool SetGameState( const Socket::Packet& inPacket );

		// Try to determine a good move that could be made by the given participant with the current game state that is based on various amounts of information.
		bool FindGoodMoveForParticipant( int color, int& sourceID, int& destinationID );
		bool FindGoodMoveForParticipant( int color, int& sourceID, int& destinationID, int turnCount, MoveMemory& moveMemory );

		// Tell us if any piece is still animating.
		bool AnyPieceInMotion( void );

		//=====================================================================================
		// Each of these represents a location on the game board.
		class Location
		{
		public:

			Location( int occupant, int zone, int locationID );
			~Location( void );

			void Render( GLenum renderMode, bool highlight );

			void CalcPositionOfAdjacencies( const c3ga::vectorE3GA& position, VisitationMap& visitationMap );

			void SetAdjacency( int index, Location* location );
			Location* GetAdjacency( int index );

			int GetZone( void );
			int GetLocationID( void );

			void GetRenderColor( c3ga::vectorE3GA& renderColor );

			void SetOccupant( int occupant );
			int GetOccupant( void );

			void Visited( bool visited );
			bool Visited( void );

			void SetPiece( Piece* piece );
			Piece* GetPiece( void );

			const c3ga::vectorE3GA& GetPosition( void );

			int GetAdjacencyCount( void );

		private:

			c3ga::vectorE3GA position;
			Location* adjacency[ ADJACENCIES ];
			int occupant;
			int zone;
			int locationID;
			Piece* piece;
			bool visited;
		};

		//=====================================================================================
		// A board piece is used to show which board locations are occupied by which players.
		// These animate and lag behind the true location of each piece.
		class Piece
		{
		public:

			enum
			{
				ROTATION_RATE = 220,	// This is in units of degrees per second.
			};

			Piece( int locationID );
			~Piece( void );

			void Render( Board* board, bool highlight );
			void Animate( double frameRate );

			void ResetAnimation( const MoveSequence& moveSequence );

			int GetLocationID( void );

			bool IsInMotion( void );

		private:

			MoveSequence moveSequence;
			double pivotAngle;
		};

	private:

		void CreateGraph( void );
		void DestroyGraph( void );

		void CreatePieces( void );
		void DestroyPieces( void );

		// These are internal, because only the application of a move causes us to change who's turn it is.
		void NextTurn( void );
		void IncrementTurn( void );

		Location* FindTargetLocation( int zoneTarget );
		bool ApplyMoveInternally( int sourceID, int destinationID );
		bool ApplyMoveInternally( const Move& move );
		bool FindMoveSequenceRecursively( Location* currentLocation, Location* destinationLocation, MoveSequence& moveSequence );
		Location* FindZoneVertex( int zone );
		Location* FindClosestUnoccupiedLocationInZone( const c3ga::vectorE3GA& position, int zone );
		void FindAllMovesForParticipant( int color, SourceMap& sourceMap, int turnCount = 1 );
		DestinationMap* FindAllMovesForLocation( Location* location );
		void FindAllMovesForLocationRecursively( Location* location, DestinationMap& destinationMap );
		void DeleteMoves( SourceMap& sourceMap );

		struct DecisionBasis
		{
			int color;
			int zoneTarget;
			Location* sourceVertexLocation;
			Location* destinationVertexLocation;
			c3ga::vectorE3GA generalMoveDirection;
			Location* targetLocation;
		};

		bool IsConfinedToDiamond( const MoveList& moveList, const DecisionBasis& decisionBasis );
		bool HasDegenerateMoves( const MoveList& moveList );

		bool GenerateDecisionBasisForColor( int color, DecisionBasis& decisionBasis );

		class MoveListVisitor
		{
		public:
			virtual bool Visit( const MoveList& moveList ) = 0;
		};

		void VisitAllMoveLists( const SourceMap& sourceMap, MoveList moveList, MoveListVisitor* moveListVisitor );

		double CalculateNetMoveDistance( const MoveList& moveList, const DecisionBasis& decisionBasis );
		double CalculateNetDistanceToTargets( const MoveList& moveList, const DecisionBasis& decisionBasis );

		int participants;
		int whosTurn;
		LocationMap locationMap;
		PieceList pieceList;
		bool animate;
	};
}

// ChiCheBoard.h