// ChiCheScorePanel.h

#pragma once

#include "ChiChePanel.h"
#include <wx/listctrl.h>

namespace ChiChe
{
	class ScorePanel;
}

//=====================================================================================
class ChiChe::ScorePanel : public ChiChe::Panel
{
public:

	ScorePanel( void );
	virtual ~ScorePanel( void );

	virtual void SetupPaneInfo( wxAuiPaneInfo& paneInfo ) override;
	virtual void CreateControls( void ) override;
	virtual void Update( void ) override;

	wxListView* listView;
};

// ChiCheScorePanel.h
