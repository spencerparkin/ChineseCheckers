// ChiCheChatPanel.h

#pragma once

#include "ChiChePanel.h"
#include "ChiCheSocket.h"
#include <wx/richtext/richtextctrl.h>
#include <wx/textctrl.h>
#include <wx/button.h>

namespace ChiChe
{
	class ChatPanel;
}

//=====================================================================================
class ChiChe::ChatPanel : public ChiChe::Panel
{
public:

	ChatPanel( void );
	virtual ~ChatPanel( void );

	virtual void SetupPaneInfo( wxAuiPaneInfo& paneInfo ) override;
	virtual void CreateControls( void ) override;
	virtual void Update( void ) override;

	static bool PackChatMessage( Socket::Packet& outPacket, const wxString& message, int color );
	static bool UnpackChatMessage( const Socket::Packet& inPacket, wxString& message, int& color );

	void ReceiveChatMessage( const wxString& message, int color );

private:

	void OnTextEnter( wxEvent& event );

	wxRichTextCtrl* outputText;
	wxTextCtrl* inputText;
};

// ChiCheChatPanel.h