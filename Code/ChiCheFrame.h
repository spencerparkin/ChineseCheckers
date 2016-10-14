// ChiCheFrame.h

#pragma once

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/aui/aui.h>

namespace ChiChe
{
	class Canvas;

	//=====================================================================================
	class Frame : public wxFrame
	{
	public:

		Frame( void );
		virtual ~Frame( void );

		wxStatusBar* GetStatusBar( void );

		Canvas* GetCanvas( void );

		void PanelUpdateNeeded( void ) { panelUpdateNeeded = true; }

	private:

		enum
		{
			ID_HostGame = wxID_HIGHEST,
			ID_JoinGame,
			ID_LeaveGame,
			ID_KillGame,
			ID_ToggleSound,
			ID_Effect,
			ID_DoinkEffect,
			ID_FartEffect,
			ID_HiyawEffect,
			ID_NewProcess,
			ID_Exit,
			ID_ScorePanelToggle,
			ID_WinnerPanelToggle,
			ID_ChatPanelToggle,
			ID_About,
			ID_Timer,
		};

		void OnHostGame( wxCommandEvent& event );
		void OnJoinGame( wxCommandEvent& event );
		void OnLeaveGame( wxCommandEvent& event );
		void OnKillGame( wxCommandEvent& event );
		void OnToggleSound( wxCommandEvent& event );
		void OnToggleEffect( wxCommandEvent& event );
		void OnNewProcess( wxCommandEvent& event );
		void OnExit( wxCommandEvent& event );
		void OnAbout( wxCommandEvent& event );
		void OnUpdateMenuItemUI( wxUpdateUIEvent& event );
		void OnTimer( wxTimerEvent& event );
		void OnClose( wxCloseEvent& event );
		void OnActivate( wxActivateEvent& event );
		void OnScorePanelToggle( wxCommandEvent& event );
		void OnWinnerPanelToggle( wxCommandEvent& event );
		void OnChatPanelToggle( wxCommandEvent& event );

		void KillServer( void );
		void KillClient( void );

		void UpdateAllPanels( void );
		bool IsPanelInUse( const wxString& panelTitle, wxAuiPaneInfo** foundPaneInfo = nullptr );
		bool TogglePanel( const wxString& panelTitle );

		wxTimer timer;
		wxMenuBar* menuBar;
		wxStatusBar* statusBar;
		bool continuousRefresh;
		wxAuiManager* auiManager;
		bool panelUpdateNeeded;
	};
}

// ChiCheFrame.h