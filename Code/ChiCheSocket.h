// ChiCheSocket.h

#pragma once

#include <wx/socket.h>

namespace ChiChe
{
	//=====================================================================================
	// This class is designed to let its user read and write entire packets without
	// the need to ever block on read or write.
	class Socket
	{
	public:

		enum
		{
			BYTE_SWAP_CONSTANT = 0x00000FFFF,
		};

		Socket( wxSocketBase* socket );
		~Socket( void );

		//=====================================================================================
		// A packet is just a raw buffer of data.  The interpretation of that buffer is
		// up to the user and can be discerned with the type parameter.
		class Packet
		{
		public:

			enum Type
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

			Packet( void );
			~Packet( void );

			void Reset( void );

			void SetType( wxInt32 type );
			int GetType( void ) const;

			void SetData( const wxInt8* data );
			const wxInt8* GetData( void ) const;

			void SetSize( wxInt32 size );
			wxInt32 GetSize( void ) const;

			void OwnsMemory( bool ownsMemory );
			bool OwnsMemory( void ) const;

			void ByteSwap( bool byteSwap );
			bool ByteSwap( void ) const;

		private:

			wxInt32 type;
			const wxInt8* data;
			wxInt32 size;
			bool byteSwap;
			bool ownsMemory;
		};

		// Write a full packet to the socket without blocking.
		bool WritePacket( const Packet& packet );

		// Read an entire packet from the socket without blocking.
		bool ReadPacket( Packet& packet );

		// Advance the input and output streams of the socket.
		bool Advance( void );

		// Is the socket still connected?
		bool IsConnected( void );

	private:

		//=====================================================================================
		struct Header
		{
			wxInt32 byteSwap;
			wxInt32 type;
			wxInt32 size;
		};

		//=====================================================================================
		class Stream
		{
		public:

			Stream( void );
			~Stream( void );

			void Append( const wxInt8* data, wxInt32 size );
			void Shift( wxInt32 size );

			const wxInt8* Buffer( void ) const;
			wxInt32 BufferSize( void ) const;

		private:

			wxInt8* buffer;
			wxInt32 bufferSize;
			wxInt32 bufferAllocSize;
		};

		Stream inputStream;
		Stream outputStream;

		wxSocketBase* socket;
	};
}

// ChiCheSocket.h