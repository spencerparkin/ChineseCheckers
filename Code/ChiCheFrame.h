// ChiCheFrame.h

namespace ChiChe
{
	//=====================================================================================
	class Frame : public wxFrame
	{
	public:

		Frame( void );
		virtual ~Frame( void );

	private:

		enum
		{
			ID_HostGame = wxID_HIGHEST,
			ID_JoinGame,
			ID_Exit,
			ID_About,
			ID_Timer,
		};

		void OnHostGame( wxCommandEvent& event );
		void OnJoinGame( wxCommandEvent& event );
		void OnExit( wxCommandEvent& event );
		void OnAbout( wxCommandEvent& event );
		void OnUpdateMenuItemUI( wxUpdateUIEvent& event );
		void OnTimer( wxTimerEvent& event );
		void OnClose( wxCloseEvent& event );

		Canvas* canvas;
		wxTimer timer;
		wxMenuBar* menuBar;
		wxStatusBar* statusBar;
	};
}

// ChiCheFrame.h