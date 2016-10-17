// ChiCheChatPanel.cpp

#include "ChiCheChatPanel.h"
#include "ChiCheClient.h"
#include "ChiCheSocket.h"
#include "ChiCheApp.h"
#include <wx/sizer.h>

using namespace ChiChe;

// Now, I suppose we could have the server maintain the chat log so that
// late joiners can see the conversation that has been taking place, but
// I'm not that motivated to put that in.  I think this is fine for now.

//=====================================================================================
ChatPanel::ChatPanel( void )
{
	outputText = nullptr;
	inputText = nullptr;
}

//=====================================================================================
/*virtual*/ ChatPanel::~ChatPanel( void )
{
}

//=====================================================================================
/*virtual*/ void ChatPanel::SetupPaneInfo( wxAuiPaneInfo& paneInfo )
{
	paneInfo.MinSize( 150, -1 );
}

//=====================================================================================
/*virtual*/ void ChatPanel::CreateControls( void )
{
	outputText = new wxRichTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, -1 ), wxRE_MULTILINE | wxRE_READONLY );
	inputText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1, 20 ), wxTE_PROCESS_ENTER );

	outputText->SetBackgroundColour( wxColour( 0, 0, 0 ) );
	outputText->SetForegroundColour( wxColour( 255, 255, 255 ) );

	wxBoxSizer* boxSizer = new wxBoxSizer( wxVERTICAL );
	boxSizer->Add( outputText, 1, wxGROW | wxALL, 0 );
	boxSizer->Add( inputText, 0, wxGROW | wxALL, 0 );

	SetSizer( boxSizer );

	Bind( wxEVT_TEXT_ENTER, &ChatPanel::OnTextEnter, this );
}

//=====================================================================================
/*virtual*/ void ChatPanel::Update( void )
{
}

//=====================================================================================
void ChatPanel::OnTextEnter( wxEvent& event )
{
	Client* client = wxGetApp().GetClient();
	if( !client )
		return;

	int color = client->GetColor();

	wxString message = inputText->GetValue();
	inputText->Clear();

	Socket::Packet outPacket;
	PackChatMessage( outPacket, message, color );

	client->GetSocket()->WritePacket( outPacket );
}

//=====================================================================================
void ChatPanel::ReceiveChatMessage( const wxString& message, int color )
{
	c3ga::vectorE3GA renderColor;
	Board::RenderColor( color, renderColor );

	unsigned char r = ( unsigned char )( renderColor.get_e1() * 255.0 );
	unsigned char g = ( unsigned char )( renderColor.get_e2() * 255.0 );
	unsigned char b = ( unsigned char )( renderColor.get_e3() * 255.0 );
	wxColour textRenderColor( r, g, b );

	wxString textColor;
	Board::ParticipantText( color, textColor );

	outputText->BeginBold();
	outputText->BeginTextColour( textRenderColor );
	outputText->WriteText( textColor + ": " );
	outputText->EndTextColour();
	outputText->EndBold();
	outputText->BeginTextColour( wxColour( 255, 255, 255 ) );
	outputText->WriteText( message );
	outputText->EndTextColour();
	outputText->Newline();
}

//=====================================================================================
/*static*/ bool ChatPanel::PackChatMessage( Socket::Packet& outPacket, const wxString& message, int color )
{
	int messageLength = message.Length();
	wxInt32* data = new wxInt32[ 1 + messageLength ];
	data[0] = color;

	for( int i = 0; i < messageLength; i++ )
		data[ i + 1 ] = message.GetChar( i );

	outPacket.Reset();
	outPacket.SetType( Socket::Packet::CHAT_MESSAGE );
	outPacket.SetData( ( wxInt8* )data );
	outPacket.SetSize( ( 1 + messageLength ) * sizeof( wxInt32 ) );
	outPacket.OwnsMemory( true );

	return true;
}

//=====================================================================================
/*static*/ bool ChatPanel::UnpackChatMessage( const Socket::Packet& inPacket, wxString& message, int& color )
{
	if( inPacket.GetType() != Socket::Packet::CHAT_MESSAGE )
		return false;

	const wxInt32* data = ( const wxInt32* )inPacket.GetData();

	if( inPacket.ByteSwap() )
		color = wxINT32_SWAP_ALWAYS( data[0] );
	else
		color = data[0];

	message = "";
	int messageLength = ( inPacket.GetSize() - 1 ) / sizeof( wxInt32 );
	for( int i = 1; i <= messageLength; i++ )
	{
		wxInt32 word = inPacket.ByteSwap() ? wxINT32_SWAP_ALWAYS( data[i] ) : data[i];
		wxUniChar uniChar( word );
		message.Append( uniChar );
	}

	return true;
}

// ChiCheChatPanel.cpp