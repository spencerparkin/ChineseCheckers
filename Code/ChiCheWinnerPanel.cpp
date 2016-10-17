// ChiCheWinnerPanel.cpp

#include "ChiCheWinnerPanel.h"
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/datetime.h>

using namespace ChiChe;

//=====================================================================================
WinnerPanel::WinnerPanel( void )
{
	queryComboBox = nullptr;
	maxResultsSpin = nullptr;
	refreshButton = nullptr;
	queryResultsView = nullptr;
}

//=====================================================================================
/*virtual*/ WinnerPanel::~WinnerPanel( void )
{
}

//=====================================================================================
/*virtual*/ void WinnerPanel::SetupPaneInfo( wxAuiPaneInfo& paneInfo )
{
	paneInfo.MinSize( 150, -1 );
}

//=====================================================================================
/*virtual*/ void WinnerPanel::CreateControls( void )
{
	queryComboBox = new wxComboBox( this, wxID_ANY );
	maxResultsSpin = new wxSpinCtrl( this, wxID_ANY );
	refreshButton = new wxButton( this, wxID_ANY, wxT( "Refresh" ) );
	wxBoxSizer* headerBoxSizer = new wxBoxSizer( wxHORIZONTAL );
	headerBoxSizer->Add( queryComboBox, 1, wxGROW | wxALL, 4 );
	headerBoxSizer->Add( maxResultsSpin, 0, wxALL, 4 );
	headerBoxSizer->Add( refreshButton, 0, wxALL, 4 );

	queryResultsView = new wxDataViewCtrl( this, wxID_ANY );
	wxBoxSizer* boxSizer = new wxBoxSizer( wxVERTICAL );
	boxSizer->Add( headerBoxSizer, 0, 0 );
	boxSizer->Add( queryResultsView, 1, wxGROW );
	SetSizer( boxSizer );

	maxResultsSpin->SetRange( 1, 300 );
	maxResultsSpin->SetValue( 20 );

	wxObjectDataPtr< WinEntryDataViewModel > winEntryDataViewModel;
	winEntryDataViewModel = new WinEntryDataViewModel();
	queryResultsView->AssociateModel( winEntryDataViewModel.get() );

	queryComboBox->Insert( "High Scores", 0 );
	queryComboBox->Insert( "Fastest Wins", 1 );
	queryComboBox->SetValue( "High Scores" );

	queryResultsView->AppendColumn( new wxDataViewColumn( "Winner", new wxDataViewTextRenderer(), 0 ) );
	queryResultsView->AppendColumn( new wxDataViewColumn( "Score", new wxDataViewTextRenderer(), 1 ) );
	queryResultsView->AppendColumn( new wxDataViewColumn( "Turns Taken", new wxDataViewTextRenderer(), 2 ) );
	queryResultsView->AppendColumn( new wxDataViewColumn( "Date of Win", new wxDataViewDateRenderer(), 3 ) );

	refreshButton->Bind( wxEVT_BUTTON, &WinnerPanel::OnRefreshButtonPressed, this );
	queryComboBox->Bind( wxEVT_COMBOBOX, &WinnerPanel::OnComboBoxSelectionChanged, this );
}

//=====================================================================================
/*virtual*/ void WinnerPanel::Update( void )
{
	if( queryResultsView )
		queryResultsView->GetModel()->Cleared();
}

//=====================================================================================
bool WinnerPanel::ExecuteQuery( void )
{
	bool success = false;
	Mongo* mongo = nullptr;
	
	do
	{
		// This causes the model to re-sense the data, but more importantly, I'm hoping this
		// invalidates any pointers the model is hanging on to as they are about to become stale.
		queryResultsView->GetModel()->Cleared();

		mongo = new Mongo();

		if( !mongo->Connect() )
		{
			wxMessageBox( "Failed to connect to mongo database.", "Error", wxICON_ERROR | wxCENTRE );
			break;
		}

		int winEntryListSize = maxResultsSpin->GetValue();

		WinEntryDataViewModel* winEntryDataViewModel = ( WinEntryDataViewModel* )queryResultsView->GetModel();

		wxString whichQuery = queryComboBox->GetValue();
		if( whichQuery == "High Scores" )
		{
			if( !mongo->GetTopHighScoresList( winEntryDataViewModel->winEntryList, winEntryListSize ) )
				break;
		}
		else if( whichQuery == "Fastest Wins" )
		{
			if( !mongo->GetFastestWinList( winEntryDataViewModel->winEntryList, winEntryListSize ) )
				break;
		}
		else
			break;

		mongo->Disconnect();

		success = true;
	}
	while( false );

	if( !success )
	{
		wxString error = mongo->GetError();
		if( error.Length() > 0 )
			wxMessageBox( "Mongo Error: " + error, "Error", wxICON_ERROR | wxCENTRE );
	}
	
	delete mongo;

	return success;
}

//=====================================================================================
void WinnerPanel::OnRefreshButtonPressed( wxCommandEvent& event )
{
	ExecuteQuery();
	Update();
}

//=====================================================================================
void WinnerPanel::OnComboBoxSelectionChanged( wxCommandEvent& event )
{
	ExecuteQuery();
	Update();
}

//=====================================================================================
WinnerPanel::WinEntryDataViewModel::WinEntryDataViewModel( void )
{
}

//=====================================================================================
/*virtual*/ WinnerPanel::WinEntryDataViewModel::~WinEntryDataViewModel( void )
{
	Mongo::FreeWinEntryList( winEntryList );
}

//=====================================================================================
/*virtual*/ bool WinnerPanel::WinEntryDataViewModel::IsContainer( const wxDataViewItem& item ) const
{
	if( !item.IsOk() )
		return true;

	return false;
}

//=====================================================================================
/*virtual*/ wxDataViewItem WinnerPanel::WinEntryDataViewModel::GetParent( const wxDataViewItem& item ) const
{
	wxDataViewItem parentItem;
	return parentItem;
}

//=====================================================================================
/*virtual*/ unsigned int WinnerPanel::WinEntryDataViewModel::GetChildren( const wxDataViewItem& item, wxDataViewItemArray& children ) const
{
	if( item.IsOk() )
		return 0;

	for( Mongo::WinEntryList::const_iterator iter = winEntryList.cbegin(); iter != winEntryList.cend(); iter++ )
	{
		const Mongo::WinEntry* winEntry = *iter;

		wxDataViewItem item;
		item.m_pItem = ( void* )winEntry;

		children.Add( item );
	}

	return winEntryList.size();
}

//=====================================================================================
/*virtual*/ unsigned int WinnerPanel::WinEntryDataViewModel::GetColumnCount( void ) const
{
	return 4;
}

//=====================================================================================
/*virtual*/ wxString WinnerPanel::WinEntryDataViewModel::GetColumnType( unsigned int col ) const
{
	switch( col )
	{
		case 0:
		{
			wxVariant variant( "" );
			return variant.GetType();
		}
		case 1:
		case 2:
		{
			wxVariant variant( 1 );
			return variant.GetType();
		}
		case 3:
		{
			wxDateTime dateTime;
			wxVariant variant( dateTime );
			return variant.GetType();
		}
	}

	return "";
}

//=====================================================================================
/*virtual*/ void WinnerPanel::WinEntryDataViewModel::GetValue( wxVariant& variant, const wxDataViewItem& item, unsigned int col ) const
{
	const Mongo::WinEntry* winEntry = ( const Mongo::WinEntry* )item.m_pItem;

	switch( col )
	{
		case 0:
		{
			variant = winEntry->winnerName;
			break;
		}
		case 1:
		{
			variant = winEntry->score;
			break;
		}
		case 2:
		{
			variant = ( long )winEntry->turnCount;
			break;
		}
		case 3:
		{
			variant = winEntry->dateOfWin;
			break;
		}
	}
}

//=====================================================================================
/*virtual*/ bool WinnerPanel::WinEntryDataViewModel::SetValue( const wxVariant& variant, const wxDataViewItem& item, unsigned int col )
{
	return false;
}

// ChiCheWinnerPanel.cpp