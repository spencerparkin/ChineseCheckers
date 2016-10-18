// ChiCheCanvasPanel.cpp

#include "ChiCheCanvasPanel.h"
#include "ChiCheCanvas.h"
#include <wx/sizer.h>

using namespace ChiChe;

//=====================================================================================
CanvasPanel::CanvasPanel( void )
{
	canvas = nullptr;
}

//=====================================================================================
/*virtual*/ CanvasPanel::~CanvasPanel( void )
{
}

//=====================================================================================
/*virtual*/ void CanvasPanel::SetupPaneInfo( wxAuiPaneInfo& paneInfo )
{
	paneInfo.CenterPane();
}

//=====================================================================================
/*virtual*/ void CanvasPanel::CreateControls( void )
{
	canvas = new Canvas( this );

	wxBoxSizer* boxSizer = new wxBoxSizer( wxHORIZONTAL );
	boxSizer->Add( canvas, 1, wxALL | wxGROW, 0 );
	SetSizer( boxSizer );
}

//=====================================================================================
/*virtual*/ void CanvasPanel::Update( void )
{
}

// ChiCheCanvasPanel.cpp