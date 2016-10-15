// ChiCheWinnerPanel.h

#pragma once

#include "ChiChePanel.h"
#include "ChiCheMongo.h"
#include <wx/combobox.h>
#include <wx/dataview.h>
#include <wx/button.h>
#include <wx/spinctrl.h>

namespace ChiChe
{
	class WinnerPanel;
}

//=====================================================================================
class ChiChe::WinnerPanel : public ChiChe::Panel
{
public:

	WinnerPanel( void );
	virtual ~WinnerPanel( void );

	virtual void SetupPaneInfo( wxAuiPaneInfo& paneInfo ) override;
	virtual void CreateControls( void ) override;
	virtual void Update( void ) override;

	class WinEntryDataViewModel : public wxDataViewModel
	{
	public:

		WinEntryDataViewModel( void );
		virtual ~WinEntryDataViewModel( void );

		virtual bool IsContainer( const wxDataViewItem& item ) const override;
		virtual wxDataViewItem GetParent( const wxDataViewItem& item ) const override;
		virtual unsigned int GetChildren( const wxDataViewItem& item, wxDataViewItemArray& children ) const override;
		virtual unsigned int GetColumnCount( void ) const override;
		virtual wxString GetColumnType( unsigned int col ) const override;
		virtual void GetValue( wxVariant& varient, const wxDataViewItem& item, unsigned int col ) const override;
		virtual bool SetValue( const wxVariant& variant, const wxDataViewItem& item, unsigned int col ) override;

		Mongo::WinEntryList winEntryList;
	};

private:

	bool ExecuteQuery( void );

	void OnRefreshButtonPressed( wxCommandEvent& event );
	void OnComboBoxSelectionChanged( wxCommandEvent& event );

	wxComboBox* queryComboBox;
	wxSpinCtrl* maxResultsSpin;
	wxButton* refreshButton;
	wxDataViewCtrl* queryResultsView;
	
};

// ChiCheWinnerPanel.h
