// ChiCheScorePanel.cpp

#include "ChiCheScorePanel.h"
#include "ChiCheApp.h"
#include "ChiCheClient.h"
#include "ChiCheBoard.h"
#include <wx/sizer.h>

using namespace ChiChe;

//=====================================================================================
ScorePanel::ScorePanel( void )
{
	listView = nullptr;
}

//=====================================================================================
/*virtual*/ ScorePanel::~ScorePanel( void )
{
}

//=====================================================================================
/*virtual*/ void ScorePanel::SetupPaneInfo( wxAuiPaneInfo& paneInfo )
{
	paneInfo.MinSize( 100, 300 );
}

//=====================================================================================
/*virtual*/ void ScorePanel::CreateControls( void )
{
	listView = new wxListView( this );
	listView->AppendColumn( "Color" );
	listView->AppendColumn( "Score" );

	wxBoxSizer* boxSizer = new wxBoxSizer( wxVERTICAL );
	boxSizer->Add( listView, 1, wxGROW | wxALL );
	SetSizer( boxSizer );
}

//=====================================================================================
/*virtual*/ void ScorePanel::Update( void )
{
	listView->DeleteAllItems();

	Client* client = wxGetApp().GetClient();
	if( !client )
		return;

	Board* board = client->GetBoard();
	int i = 0;
	for( int color = Board::RED; color <= Board::CYAN; color++ )
	{
		long score = 0;
		if( board->GetScore( color, score ) )
		{
			wxString textColor;
			board->ParticipantText( color, textColor );
			listView->InsertItem( i, textColor );
			listView->SetItem( i, 1, wxString::Format( "%ld", score ) );
			i++;
		}
	}
}

// ChiCheScorePanel.cpp
