// ChiChePanel.h

#pragma once

#include <wx/panel.h>
#include <wx/aui/aui.h>

namespace ChiChe
{
	class Panel;
}

//=====================================================================================
class ChiChe::Panel : public wxPanel
{
public:

	Panel( void );
	virtual ~Panel( void );

	virtual void SetupPaneInfo( wxAuiPaneInfo& paneInfo );
	virtual void CreateControls( void ) = 0;
	virtual void Update( void ) = 0;
};

// ChiChePanel.h
