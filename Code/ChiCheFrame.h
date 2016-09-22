// ChiCheFrame.h

namespace ChiChe
{
	//=====================================================================================
	class Frame : public wxFrame
	{
	public:

		Frame( void );
		virtual ~Frame( void );

		wxStatusBar* GetStatusBar( void );

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
			ID_Exit,
			ID_About,
			ID_Timer,
		};

		void OnHostGame( wxCommandEvent& event );
		void OnJoinGame( wxCommandEvent& event );
		void OnLeaveGame( wxCommandEvent& event );
		void OnKillGame( wxCommandEvent& event );
		void OnToggleSound( wxCommandEvent& event );
		void OnToggleEffect( wxCommandEvent& event );
		void OnExit( wxCommandEvent& event );
		void OnAbout( wxCommandEvent& event );
		void OnUpdateMenuItemUI( wxUpdateUIEvent& event );
		void OnTimer( wxTimerEvent& event );
		void OnClose( wxCloseEvent& event );
		void OnActivate( wxActivateEvent& event );

		void KillServer( void );
		void KillClient( void );

		Canvas* canvas;
		wxTimer timer;
		wxMenuBar* menuBar;
		wxStatusBar* statusBar;
		bool continuousRefresh;
	};
}

// ChiCheFrame.h