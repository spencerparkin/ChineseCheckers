// ChiCheCanvasPanel.h

#pragma once

#include "ChiChePanel.h"

namespace ChiChe
{
	class Canvas;
	class CanvasPanel;
}

//=====================================================================================
class ChiChe::CanvasPanel : public ChiChe::Panel
{
public:

	CanvasPanel( void );
	virtual ~CanvasPanel( void );

	virtual void SetupPaneInfo( wxAuiPaneInfo& paneInfo ) override;
	virtual void CreateControls( void ) override;
	virtual void Update( void ) override;

	Canvas* canvas;
};

// ChiCheCanvasPanel.h
