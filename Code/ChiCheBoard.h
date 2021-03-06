// ChiCheBoard.h

#pragma once

#include "ChiCheSocket.h"
#include "c3ga/c3ga.h"
#include <wx/glcanvas.h>
#include <wx/event.h>
#include <list>
#include <map>

namespace ChiChe
{
	//=====================================================================================
	// This represents the game board, which is the entire state of the game, including
	// who's turn it is, and where all the player pieces are located.
	class Board
	{
		friend class Brain;

	public:

		//=====================================================================================
		class Event : public wxEvent
		{
		public:

			Event( int winid = 0, wxEventType commandType = wxEVT_NULL );
			virtual ~Event( void );

			virtual wxEvent* Clone( void ) const override;

			void SetMessage( const wxString message );
			wxString GetMessage( void ) const;

		private:

			wxString message;
		};

		//=================================================================================
		static const wxEventTypeTag< Event > BEGIN_BOARD_THINKING;
		static const wxEventTypeTag< Event > UPDATE_BOARD_THINKING;
		static const wxEventTypeTag< Event > END_BOARD_THINKING;

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

			void Inverse( Move& inverseMove ) const;
		};

		typedef std::list< Move > MoveList;		// This is what one might also call a move sequence.  It is a sequence of moves made over one or more turns.
		typedef std::list< int > MoveSequence;	// Despite the name, this is just a sequence of moves that can be made in a single turn.
		typedef std::map< int, Location* > LocationMap;
		typedef std::list< Location* > LocationList;
		typedef std::list< Piece* > PieceList;
		typedef std::map< int, bool > VisitationMap;
		typedef std::map< int, int > TargetCountMap;
		typedef std::map< int, long > ScoreMap;
		typedef std::map< int, int > TurnCountMap;

		// Construct a game board with the given participants.
		Board( int participants, bool animate );
		~Board( void );

		// Provide access to the board's event handler.
		void SetEventHandler( wxEvtHandler* eventHandler );
		wxEvtHandler* GetEventHandler( void );

		// Which zone are pieces of the given color trying to get to?
		static int ZoneTarget( int color );

		// Return the color of the given zone/occupant/etc.
		static void RenderColor( int color, c3ga::vectorE3GA& renderColor );

		// Return the color of the given zone/occupant/etc. as a string.
		static void ParticipantText( int color, wxString& textColor );

		// Grab a game move, if any, found in the given packet.
		static bool UnpackMove( const Socket::Packet& inPacket, wxInt32& sourceID, wxInt32& destinationID );

		// Put a game move into the given packet.
		static bool PackMove( Socket::Packet& outPacket, wxInt32 sourceID, wxInt32 destinationID );

		// Pack and unpack score bonuses!
		static bool UnpackScoreBonus( const Socket::Packet& inPacket, wxInt32& participant, wxInt64& scoreBonus );
		static bool PackScoreBonus( Socket::Packet& outPacket, wxInt32 participant, wxInt64 scoreBonus );

		// Is the given color particpating in the game?
		bool IsParticipant( int color );

		// Is it the given participant's turn?
		bool IsParticipantsTurn( int color );

		// Whos turn is it?
		int WhosTurn( void );

		// Get the participants bit field.
		int GetParticipants( void );

		// Return how many opponents the given color faces.
		int GetOpponentCount( int color );

		// Return the occupant at the given location.
		int OccupantAtLocation( int locationID );

		// Return the spacial position of the given board location.
		bool PositionAtLocation( int locationID, c3ga::vectorE3GA& position );

		// Return the winning piece color, if any.
		int DetermineWinner( void );

		// Calculate the occupancy for each participant in their respective zones.
		void ComputeTargetCountMap( TargetCountMap& targetCountMap );

		// Render the board for display or selection.
		void Render( GLenum renderMode, int highlightLocationID );

		// Advance the position of each board piece by one frame's worth of animation.
		void Animate( double frameRate );

		// Keep score as each given move is made.
		void AccumulateScoreForMove( const Move& move, int participant, int jumpCount );

		// Tell us if the given locations are adjacent.  If not, tell us if they have a mutual adjacency.
		bool LocationsAreAdjacent( Location* locationA, Location* locationB, Location** mutualAdjacency = nullptr );

		// Return the location ID of the location selected by the user, if any.
		int FindSelectedLocation( unsigned int* hitBuffer, int hitBufferSize, int hitCount );

		// If a valid sequence of moves exists from the given source location to the given destination location, return it as a move sequence.
		// Note that this also takes into account who's turn it is.
		bool FindMoveSequence( int sourceID, int destinationID, MoveSequence& moveSequence );
		bool FindMoveSequence( const Move& move, MoveSequence& moveSequence );

		// Change the state of the game board by the given move sequence.  This will also change who's turn it is.
		bool ApplyMoveSequence( const MoveSequence& moveSequence, bool applyBonuses );

		// Get bonus points to the given participant!
		bool ApplyScoreBonus( int participant, long scoreBonus );

		// Access the score for the given participant.
		bool GetScore( int participant, long& score );
		bool SetScore( int participant, long score );

		// Package the state of the entire game into a packet.
		bool GetGameState( Socket::Packet& outPacket );

		// Internalize the entire state of the game that is in the given packet.
		bool SetGameState( const Socket::Packet& inPacket );

		// Tell us if any piece is still animating.
		bool AnyPieceInMotion( void );

		// Return the list of locations of all participants of the given color.
		void FindParticpantLocations( int color, LocationList& locationList );

		// Return in the given list the set of all possible locations that can be reached by the given location.
		void FindAllPossibleDestinations( Location* sourceLocation, LocationList& destinationLocationList, bool noIntermediates );
		void FindAllPossibleDestinationsRecursively( Location* currentLocation, LocationList& destinationLocationList, bool noIntermediates, int depth );

		// Allocate and return a copy of this board.
		Board* Clone( void ) const;

		// Compute and return the total distance of each occupant of the given color to the given location.
		double CalculateTotalDistanceToLocation( int color, const Location* location );

		// Compute and return the distance moved by the given move.
		double CalculateMoveDistance( const Move& move );

		// These are used to save and restore test cases for faster bug reproduction.
		bool SaveToXML( const wxString& xmlFile );
		bool LoadFromXML( const wxString& xmlFile );

		// Tell us how many turns the given participant has taken.
		int GetTurnCount( int participant );

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
			int GetLocationID( void ) const;

			void GetRenderColor( c3ga::vectorE3GA& renderColor );

			void SetOccupant( int occupant );
			int GetOccupant( void ) const;

			void Visited( bool visited );
			bool Visited( void );

			void SetPiece( Piece* piece );
			Piece* GetPiece( void );

			const c3ga::vectorE3GA& GetPosition( void ) const;

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
			void Animate( double frameRate, Board* board );

			void ResetAnimation( const MoveSequence& moveSequence );

			int GetLocationID( void );

			bool IsInMotion( void );

			int GetJumpCount( void );

		private:

			MoveSequence moveSequence;
			double pivotAngle;
			int jumpCount;
		};

	private:

		void CreateGraph( void );
		void DestroyGraph( void );

		void CreatePieces( void );
		void DestroyPieces( void );

		// These are internal, because only the application of a move causes us to change who's turn it is.
		void NextTurn( void );
		void IncrementTurn( void );

		bool ApplyMoveInternally( int sourceID, int destinationID );
		bool ApplyMoveInternally( const Move& move );
		bool FindMoveSequenceRecursively( Location* currentLocation, Location* destinationLocation, MoveSequence& moveSequence );

		void SendEvent( wxEventType eventType, const wxString message );

		wxEvtHandler* eventHandler;
		int participants;
		int whosTurn;
		LocationMap locationMap;
		PieceList pieceList;
		bool animate;
		ScoreMap scoreMap;
		TurnCountMap turnCountMap;
	};
}

// ChiCheBoard.h