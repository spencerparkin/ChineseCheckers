// ChiCheWinnerPanel.cpp

#include "ChiCheWinnerPanel.h"
#include <wx/sizer.h>
#include <wx/msgdlg.h>

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

	delete mongo;

	return success;
}

//=====================================================================================
void WinnerPanel::OnRefreshButtonPressed( wxCommandEvent& event )
{
	ExecuteQuery();
}

//=====================================================================================
void WinnerPanel::OnComboBoxSelectionChanged( wxCommandEvent& event )
{
	ExecuteQuery();
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
	return 0;
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
		case 0: return "String";
		case 1: return "Integer";
		case 2: return "Integer";
		case 3: return "Date";
	}

	return "?";
}

//=====================================================================================
/*virtual*/ void WinnerPanel::WinEntryDataViewModel::GetValue( wxVariant& varient, const wxDataViewItem& item, unsigned int col ) const
{
	// TODO: Feed win list to the view control.
}

//=====================================================================================
/*virtual*/ bool WinnerPanel::WinEntryDataViewModel::SetValue( const wxVariant& variant, const wxDataViewItem& item, unsigned int col )
{
	return false;
}

// ChiCheWinnerPanel.cpp