// ChiCheSocket.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Socket::Socket( wxSocketBase* socket )
{
	this->socket = socket;
}

//=====================================================================================
Socket::~Socket( void )
{
	delete socket;
}

//=====================================================================================
bool Socket::WritePacket( const Packet& packet )
{
	Header header;
	header.byteSwap = BYTE_SWAP_CONSTANT;
	header.size = packet.GetSize();
	header.type = packet.GetType();

	outputStream.Append( ( const wxInt8* )&header, sizeof( Header ) );
	outputStream.Append( packet.GetData(), packet.GetSize() );

	return true;
}

//=====================================================================================
bool Socket::ReadPacket( Packet& packet )
{
	if( inputStream.BufferSize() < sizeof( Header ) )
		return false;

	Header header = *( const Header* )inputStream.Buffer();
	bool byteSwap = ( header.byteSwap != BYTE_SWAP_CONSTANT ) ? true : false;
	if( byteSwap )
	{
		header.byteSwap = wxINT32_SWAP_ALWAYS( header.byteSwap );
		header.size = wxINT32_SWAP_ALWAYS( header.size );
		header.type = wxINT32_SWAP_ALWAYS( header.type );
	}

	wxInt32 packetSize = sizeof( Header ) + header.size;
	if( inputStream.BufferSize() < packetSize )
		return false;

	wxInt8* data = new wxInt8[ header.size ];
	memcpy( data, inputStream.Buffer() + sizeof( Header ), header.size );
	packet.SetData( data );
	packet.OwnsMemory( true );
	packet.SetSize( header.size );
	packet.ByteSwap( byteSwap );
	packet.SetType( header.type );

	inputStream.Shift( packetSize );

	return true;
}

//=====================================================================================
bool Socket::Advance( void )
{
	if( !socket->IsConnected() )
		return false;

	wxInt8 buffer[ 1024 ];
	wxInt32 size = sizeof( buffer ) / sizeof( wxInt8 );
	socket->Read( buffer, size );
	if( !socket->Error() )
	{
		size = ( signed )socket->LastReadCount();
		if( size > 0 )
			inputStream.Append( buffer, size );
	}

	size = outputStream.BufferSize();
	if( size > 0 )
	{
		socket->Write( outputStream.Buffer(), size );
		if( !socket->Error() )
		{
			size = ( signed )socket->LastWriteCount();
			outputStream.Shift( size );
		}
	}

	return true;
}

//=====================================================================================
bool Socket::IsConnected( void )
{
	return socket->IsConnected();
}

//=====================================================================================
Socket::Stream::Stream( void )
{
	buffer = 0;
	bufferSize = 0;
	bufferAllocSize = 0;
}

//=====================================================================================
Socket::Stream::~Stream( void )
{
	delete[] buffer;
}

//=====================================================================================
void Socket::Stream::Append( const wxInt8* data, wxInt32 size )
{
	wxInt32 room = bufferAllocSize - bufferSize;
	if( room < size )
	{
		bufferAllocSize += size - room;
		wxInt8* newBuffer = new wxInt8[ bufferAllocSize ];
		memcpy( newBuffer, buffer, bufferSize );
		delete[] buffer;
		buffer = newBuffer;
	}

	memcpy( buffer + bufferSize, data, size );
	bufferSize += size;
}

//=====================================================================================
void Socket::Stream::Shift( wxInt32 size )
{
	if( size > bufferSize )
		size = bufferSize;

	bufferSize -= size;
	for( int offset = 0; offset < bufferSize; offset++ )
		buffer[ offset ] = buffer[ offset + size ];
}

//=====================================================================================
const wxInt8* Socket::Stream::Buffer( void ) const
{
	return buffer;
}

//=====================================================================================
int Socket::Stream::BufferSize( void ) const
{
	return bufferSize;
}

//=====================================================================================
Socket::Packet::Packet( void )
{
	type = 0;
	data = 0;
	size = 0;
	byteSwap = false;
	ownsMemory = false;
}

//=====================================================================================
Socket::Packet::~Packet( void )
{
	Reset();
}

//=====================================================================================
void Socket::Packet::Reset( void )
{
	if( ownsMemory )
		delete[] data;

	type = 0;
	data = 0;
	size = 0;
	byteSwap = false;
	ownsMemory = false;
}

//=====================================================================================
void Socket::Packet::SetType( wxInt32 type )
{
	this->type = type;
}

//=====================================================================================
wxInt32 Socket::Packet::GetType( void ) const
{
	return type;
}

//=====================================================================================
void Socket::Packet::SetData( const wxInt8* data )
{
	this->data = data;
}

//=====================================================================================
const wxInt8* Socket::Packet::GetData( void ) const
{
	return data;
}

//=====================================================================================
void Socket::Packet::SetSize( wxInt32 size )
{
	this->size = size;
}

//=====================================================================================
wxInt32 Socket::Packet::GetSize( void ) const
{
	return size;
}

//=====================================================================================
void Socket::Packet::OwnsMemory( bool ownsMemory )
{
	this->ownsMemory = ownsMemory;
}

//=====================================================================================
bool Socket::Packet::OwnsMemory( void ) const
{
	return ownsMemory;
}

//=====================================================================================
void Socket::Packet::ByteSwap( bool byteSwap )
{
	this->byteSwap = byteSwap;
}

//=====================================================================================
bool Socket::Packet::ByteSwap( void ) const
{
	return byteSwap;
}

// ChiCheSocket.cpp