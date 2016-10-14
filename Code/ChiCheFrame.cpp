// ChiCheFrame.cpp

#include "ChiCheFrame.h"
#include "ChiCheApp.h"
#include "ChiCheCanvas.h"
#include "ChiCheServer.h"
#include "ChiCheClient.h"
#include "ChiCheSound.h"
#include "ChiChePanel.h"
#include "ChiCheCanvasPanel.h"
#include "ChiCheScorePanel.h"
#include "ChiCheWinnerPanel.h"
#include <wx/menu.h>
#include <wx/aboutdlg.h>
#include <wx/choicdlg.h>
#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/tokenzr.h>

using namespace ChiChe;

//=====================================================================================
Frame::Frame( void ) : wxFrame( 0, wxID_ANY, "Chinese Checkers", wxDefaultPosition, wxSize( 900, 600 ) ), timer( this, ID_Timer )
{
	panelUpdateNeeded = false;

	auiManager = new wxAuiManager( this, wxAUI_MGR_LIVE_RESIZE | wxAUI_MGR_DEFAULT );

	wxMenu* gameMenu = new wxMenu();
	wxMenuItem* joinGameMenuItem = new wxMenuItem( gameMenu, ID_JoinGame, wxT( "Join Game" ), wxT( "Join a hosted game on the network." ) );
	wxMenuItem* hostGameMenuItem = new wxMenuItem( gameMenu, ID_HostGame, wxT( "Host Game" ), wxT( "Host a game on the network." ) );
	wxMenuItem* leaveGameMenuItem = new wxMenuItem( gameMenu, ID_LeaveGame, wxT( "Leave Game" ), wxT( "Disconnect from a joined game." ) );
	wxMenuItem* killGameMenuItem = new wxMenuItem( gameMenu, ID_KillGame, wxT( "Kill Game" ), wxT( "Discontinue a hosted game." ) );
	wxMenuItem* toggleSoundMenuItem = new wxMenuItem( gameMenu, ID_ToggleSound, wxT( "Sound" ), wxT( "Toggle the playing of sound FX." ), wxITEM_CHECK );
	wxMenuItem* effectMenuItem = new wxMenuItem( gameMenu, ID_Effect, wxT( "Effect" ), wxT( "Choose your sound effect." ) );
	wxMenuItem* newProcessMenuItem = new wxMenuItem( gameMenu, ID_NewProcess, wxT( "New Process" ), wxT( "Invoke a new Chinese Checkers process." ) );
	wxMenuItem* exitMenuItem = new wxMenuItem( gameMenu, ID_Exit, wxT( "Exit" ), wxT( "Exit the program." ) );

	wxMenu* effectMenu = new wxMenu();
	wxMenuItem* doinkMenuItem = new wxMenuItem( effectMenu, ID_DoinkEffect, wxT( "Doinks" ), wxT( "Your marble pieces \"doink\" about the board.  It's awesome." ), wxITEM_CHECK );
	wxMenuItem* fartMenuItem = new wxMenuItem( effectMenu, ID_FartEffect, wxT( "Farts" ), wxT( "Your marble pieces expell gas as they hop about the board." ), wxITEM_CHECK );
	wxMenuItem* hiyawMenuItem = new wxMenuItem( effectMenu, ID_HiyawEffect, wxT( "Hiyaw!" ), wxT( "Each of your marble pieces is a black-belt in karete." ), wxITEM_CHECK );
	effectMenu->Append( doinkMenuItem );
	effectMenu->Append( fartMenuItem );
	effectMenu->Append( hiyawMenuItem );
	effectMenuItem->SetSubMenu( effectMenu );

	gameMenu->Append( joinGameMenuItem );
	gameMenu->Append( hostGameMenuItem );
	gameMenu->AppendSeparator();
	gameMenu->Append( leaveGameMenuItem );
	gameMenu->Append( killGameMenuItem );
	gameMenu->AppendSeparator();
	gameMenu->Append( toggleSoundMenuItem );
	gameMenu->Append( effectMenuItem );
	gameMenu->AppendSeparator();
	gameMenu->Append( newProcessMenuItem );
	gameMenu->AppendSeparator();
	gameMenu->Append( exitMenuItem );

	wxMenu* panelMenu = new wxMenu();
	wxMenuItem* scorePanelMenuItem = new wxMenuItem( panelMenu, ID_ScorePanelToggle, wxT( "Score Panel" ), wxT( "Toggle the score panel." ), wxITEM_CHECK );
	wxMenuItem* winnerPanelMenuItem = new wxMenuItem( panelMenu, ID_WinnerPanelToggle, wxT( "High Score Panel" ), wxT( "Toggle the high-score panel." ), wxITEM_CHECK );
	wxMenuItem* chatMenuItem = new wxMenuItem( panelMenu, ID_ChatPanelToggle, wxT( "Chat Panel" ), wxT( "Toggle the chat panel" ), wxITEM_CHECK );
	panelMenu->Append( scorePanelMenuItem );
	panelMenu->Append( winnerPanelMenuItem );
	panelMenu->Append( chatMenuItem );

	wxMenu* helpMenu = new wxMenu();
	wxMenuItem* aboutMenuItem = new wxMenuItem( helpMenu, ID_About, wxT( "About" ), wxT( "Popup a dialog giving information about this program." ) );
	helpMenu->Append( aboutMenuItem );

	menuBar = new wxMenuBar();
	menuBar->Append( gameMenu, wxT( "Game" ) );
	menuBar->Append( panelMenu, wxT( "Panel" ) );
	menuBar->Append( helpMenu, wxT( "Help" ) );
	SetMenuBar( menuBar );

	statusBar = new wxStatusBar( this );
	statusBar->PushStatusText( wxT( "Welcome!  Remember to use the right mouse button to select marbles and board locations." ) );
	SetStatusBar( statusBar );

	Bind( wxEVT_MENU, &Frame::OnJoinGame, this, ID_JoinGame );
	Bind( wxEVT_MENU, &Frame::OnHostGame, this, ID_HostGame );
	Bind( wxEVT_MENU, &Frame::OnLeaveGame, this, ID_LeaveGame );
	Bind( wxEVT_MENU, &Frame::OnKillGame, this, ID_KillGame );
	Bind( wxEVT_MENU, &Frame::OnToggleSound, this, ID_ToggleSound );
	Bind( wxEVT_MENU, &Frame::OnToggleEffect, this, ID_DoinkEffect );
	Bind( wxEVT_MENU, &Frame::OnToggleEffect, this, ID_FartEffect );
	Bind( wxEVT_MENU, &Frame::OnToggleEffect, this, ID_HiyawEffect );
	Bind( wxEVT_MENU, &Frame::OnNewProcess, this, ID_NewProcess );
	Bind( wxEVT_MENU, &Frame::OnExit, this, ID_Exit );
	Bind( wxEVT_MENU, &Frame::OnAbout, this, ID_About );
	Bind( wxEVT_MENU, &Frame::OnScorePanelToggle, this, ID_ScorePanelToggle );
	Bind( wxEVT_MENU, &Frame::OnWinnerPanelToggle, this, ID_WinnerPanelToggle );
	Bind( wxEVT_MENU, &Frame::OnChatPanelToggle, this, ID_ChatPanelToggle );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_JoinGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_HostGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_LeaveGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_KillGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_ToggleSound );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_Effect );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_DoinkEffect );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_FartEffect );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_HiyawEffect );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_ScorePanelToggle );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_WinnerPanelToggle );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_ChatPanelToggle );
	Bind( wxEVT_TIMER, &Frame::OnTimer, this, ID_Timer );
	Bind( wxEVT_CLOSE_WINDOW, &Frame::OnClose, this );
	Bind( wxEVT_ACTIVATE, &Frame::OnActivate, this );

	TogglePanel( "Canvas Panel" );

	// TODO: Rememer and restore aui-manager perspectives.

	timer.Start( 50 );
	continuousRefresh = true;
}

//=====================================================================================
/*virtual*/ Frame::~Frame( void )
{
	auiManager->UnInit();
	delete auiManager;
}

//=====================================================================================
void Frame::OnHostGame( wxCommandEvent& event )
{
	Server* server = wxGetApp().GetServer();
	if( server )
		return;

	// The order of this array matches the enum in the Board class.
	wxArrayString colorChoices;
	colorChoices.Add( "Red" );
	colorChoices.Add( "Green" );
	colorChoices.Add( "Blue" );
	colorChoices.Add( "Yellow" );
	colorChoices.Add( "Magenta" );
	colorChoices.Add( "Cyan" );

	wxMultiChoiceDialog multiChoiceDialog( this, wxT( "Who will particpate in this game?" ), wxT( "Choose Participants" ), colorChoices );
	if( wxID_OK != multiChoiceDialog.ShowModal() )
		return;

	wxArrayInt selections = multiChoiceDialog.GetSelections();
	if( selections.Count() < 2 )
		return;

	int participants = 0;
	for( wxArrayInt::iterator iter = selections.begin(); iter != selections.end(); iter++ )
	{
		int color = *iter + 1;
		participants |= 1 << color;
	}

	wxScopedPtr< Server > serverPtr;
	serverPtr.reset( new Server( participants ) );

	unsigned short port = ( unsigned short )wxGetNumberFromUser( wxT( "On what port should the server listen?" ), wxT( "Port:" ), wxT( "Choose Port" ), 3000, 3000, 5000, this );
	if( !serverPtr->Initialize( port ) )
		return;

	server = serverPtr.release();
	wxGetApp().SetServer( server );

	wxMessageBox( wxT( "Your game server is online and running!" ), wxT( "Server Initialization Success" ), wxOK | wxCENTRE, this );
}

//=====================================================================================
void Frame::OnJoinGame( wxCommandEvent& event )
{
	Client* client = wxGetApp().GetClient();
	if( client )
		return;

	wxString addressString = wxT( "127.0.0.1:3000" );
	wxTextEntryDialog textEntryDialog( this, wxT( "Please enter the address of the host with port number." ), wxT( "Enter Address Of Host" ), addressString );
	if( wxID_OK != textEntryDialog.ShowModal() )
		return;

	wxStringTokenizer stringTokenizer( addressString, ":" );
	wxString ipAddressString, portString;
	if( !stringTokenizer.HasMoreTokens() )
		return;
	ipAddressString = stringTokenizer.GetNextToken();
	if( !stringTokenizer.HasMoreTokens() )
		return;
	portString = stringTokenizer.GetNextToken();

	unsigned long port;
	if( !portString.ToULong( &port ) )
		return;

	wxIPV4address address;
	if( !address.Hostname( ipAddressString ) )
		return;
	address.Service( ( unsigned short )port );

	wxArrayString typeChoices;
	typeChoices.Add( "Human" );
	typeChoices.Add( "Computer" );
	wxSingleChoiceDialog singleChoiceDialog( this, wxT( "Will this be a human or computer client?" ), wxT( "Choose Client Type" ), typeChoices );
	if( wxID_OK != singleChoiceDialog.ShowModal() )
		return;

	Client::Type clientType = Client::HUMAN;
	wxString selection = singleChoiceDialog.GetStringSelection();
	if( selection == wxT( "Computer" ) )
		clientType = Client::COMPUTER;
	
	wxScopedPtr< Client > clientPtr;
	clientPtr.reset( new Client( clientType ) );

	if( !clientPtr->Connect( address ) )
		return;

	client = clientPtr.release();
	wxGetApp().SetClient( client );
}

//=====================================================================================
void Frame::OnLeaveGame( wxCommandEvent& event )
{
	KillClient();
}

//=====================================================================================
void Frame::OnKillGame( wxCommandEvent& event )
{
	KillServer();
}

//=====================================================================================
void Frame::OnExit( wxCommandEvent& event )
{
	Close( true );
}

//=====================================================================================
void Frame::OnNewProcess( wxCommandEvent& event )
{
	wxExecute( "chiche", wxEXEC_ASYNC );
}

//=====================================================================================
void Frame::KillServer( void )
{
	Server* server = wxGetApp().GetServer();
	if( server )
	{
		server->Finalize();
		delete server;
		wxGetApp().SetServer(0);
	}
}

//=====================================================================================
void Frame::KillClient( void )
{
	Client* client = wxGetApp().GetClient();
	if( client )
	{
		delete client;
		wxGetApp().SetClient(0);
	}
}

//=====================================================================================
void Frame::OnClose( wxCloseEvent& event )
{
	KillServer();
	KillClient();

	event.Skip();
}

//=====================================================================================
void Frame::OnToggleSound( wxCommandEvent& event )
{
	wxGetApp().GetSound()->Enable( !wxGetApp().GetSound()->IsEnabled() );
}

//=====================================================================================
void Frame::OnToggleEffect( wxCommandEvent& event )
{
	switch( event.GetId() )
	{
		case ID_DoinkEffect:
		{
			wxGetApp().soundEffect = "Doink";
			break;
		}
		case ID_FartEffect:
		{
			wxGetApp().soundEffect = "Fart";
			break;
		}
		case ID_HiyawEffect:
		{
			wxGetApp().soundEffect = "Hiyaw";
			break;
		}
	}
}

//=====================================================================================
void Frame::OnAbout( wxCommandEvent& event )
{
	wxAboutDialogInfo aboutDialogInfo;
	
	aboutDialogInfo.SetName( wxT( "Chinese Checkers" ) );
	aboutDialogInfo.SetVersion( wxT( "1.0" ) );
	aboutDialogInfo.SetDescription( wxT( "This program is free software and distributed under the MIT license." ) );
	aboutDialogInfo.SetCopyright( wxT( "Copyright (C) 2013, 2016 Spencer T. Parkin <spencer.parkin@disney.com>" ) );

	wxAboutBox( aboutDialogInfo );
}

//=====================================================================================
void Frame::OnUpdateMenuItemUI( wxUpdateUIEvent& event )
{
	switch( event.GetId() )
	{
		case ID_JoinGame:
		{
			event.Enable( wxGetApp().GetClient() ? false : true );
			break;
		}
		case ID_HostGame:
		{
			event.Enable( wxGetApp().GetServer() ? false : true );
			break;
		}
		case ID_LeaveGame:
		{
			event.Enable( wxGetApp().GetClient() ? true : false );
			break;
		}
		case ID_KillGame:
		{
			event.Enable( wxGetApp().GetServer() ? true : false );
			break;
		}
		case ID_ToggleSound:
		{
			if( !wxGetApp().GetSound()->IsSetup() )
				event.Enable( false );
			else
			{
				event.Enable( true );
				event.Check( wxGetApp().GetSound()->IsEnabled() );
			}

			break;
		}
		case ID_Effect:
		{
			event.Enable( wxGetApp().GetSound()->IsSetup() && wxGetApp().GetSound()->IsEnabled() );
			break;
		}
		case ID_DoinkEffect:
		{
			event.Check( wxGetApp().soundEffect == "Doink" ? true : false );
			break;
		}
		case ID_FartEffect:
		{
			event.Check( wxGetApp().soundEffect == "Fart" ? true : false );
			break;
		}
		case ID_HiyawEffect:
		{
			event.Check( wxGetApp().soundEffect == "Hiyaw" ? true : false );
			break;
		}
		case ID_ScorePanelToggle:
		{
			event.Check( IsPanelInUse( "Score Panel" ) );
			break;
		}
		case ID_WinnerPanelToggle:
		{
			event.Check( IsPanelInUse( "Winner Panel" ) );
			break;
		}
		case ID_ChatPanelToggle:
		{
			event.Check( IsPanelInUse( "Chat Panel" ) );
			break;
		}
	}
}

//=====================================================================================
bool Frame::IsPanelInUse( const wxString& panelTitle, wxAuiPaneInfo** foundPaneInfo /*= nullptr*/ )
{
	if( foundPaneInfo )
		*foundPaneInfo = nullptr;

	wxAuiPaneInfoArray& paneInfoArray = auiManager->GetAllPanes();
	for( int i = 0; i < ( signed )paneInfoArray.GetCount(); i++ )
	{
		wxAuiPaneInfo& paneInfo = paneInfoArray[i];
		if( paneInfo.name == panelTitle )
		{
			if( foundPaneInfo )
				*foundPaneInfo = &paneInfo;
			return paneInfo.IsShown();
		}
	}

	return false;
}

//=====================================================================================
void Frame::UpdateAllPanels( void )
{
	wxAuiPaneInfoArray& paneInfoArray = auiManager->GetAllPanes();
	for( int i = 0; i < ( signed )paneInfoArray.GetCount(); i++ )
	{
		wxAuiPaneInfo& paneInfo = paneInfoArray[i];
		Panel* panel = ( Panel* )paneInfo.window;
		panel->Update();
	}
}

//=====================================================================================
void Frame::OnTimer( wxTimerEvent& event )
{
	// This routine was never meant to be re-entrant.
	// Re-entrancy could cause a crash due to client or server death.
	// If a dialog is put up in this routine, wxWidgets still calls the timer,
	// which can cause re-entrancy.
	static bool inOnTimer = false;
	if( inOnTimer )
		return;
	inOnTimer = true;

	if( panelUpdateNeeded )
	{
		UpdateAllPanels();
		panelUpdateNeeded = false;
	}

	Server* server = wxGetApp().GetServer();
	if( server && !server->Run() )
		KillServer();

	Client* client = wxGetApp().GetClient();
	if( client )
	{
		if( !client->Run() )
		{
			KillClient();
			wxMessageBox( wxT( "We have lost our connection with the server, possibly because the game server has gone down." ), wxT( "Connection Lost" ), wxOK | wxCENTRE, wxGetApp().GetFrame() );
		}
		else
		{
			client->Animate( GetCanvas()->FrameRate() );
			if( continuousRefresh )
				GetCanvas()->Refresh();
		}
	}

	inOnTimer = false;
}

//=====================================================================================
void Frame::OnActivate( wxActivateEvent& event )
{
	continuousRefresh = event.GetActive();
}

//=====================================================================================
wxStatusBar* Frame::GetStatusBar( void )
{
	return statusBar;
}

//=====================================================================================
Canvas* Frame::GetCanvas( void )
{
	wxAuiPaneInfo* foundPaneInfo;
	bool found = IsPanelInUse( "Canvas Panel", &foundPaneInfo );
	wxASSERT( found );
	CanvasPanel* canvasPanel = ( CanvasPanel* )foundPaneInfo->window;
	return canvasPanel->canvas;
}

//=====================================================================================
void Frame::OnScorePanelToggle( wxCommandEvent& event )
{
	TogglePanel( "Score Panel" );
}

//=====================================================================================
void Frame::OnWinnerPanelToggle( wxCommandEvent& event )
{
	TogglePanel( "Winner Panel" );
}

//=====================================================================================
void Frame::OnChatPanelToggle( wxCommandEvent& event )
{
	TogglePanel( "Chat Panel" );
}

//=====================================================================================
bool Frame::TogglePanel( const wxString& panelTitle )
{
	wxAuiPaneInfo* foundPaneInfo = nullptr;
	if( IsPanelInUse( panelTitle, &foundPaneInfo ) )
	{
		Panel* panel = ( Panel* )foundPaneInfo->window;
		auiManager->DetachPane( panel );
		panel->Destroy();
		auiManager->Update();
	}
	else
	{
		Panel* panel = nullptr;

		if( panelTitle == "Canvas Panel" )
			panel = new CanvasPanel();
		else if( panelTitle == "Score Panel" )
			panel = new ScorePanel();
		else if( panelTitle == "Winner Panel" )
			panel = new WinnerPanel();
		/*else if( panelTitle == "Chat Panel" )
			panel = new ChatPanel();*/

		if( panel )
		{
			panel->Create( this );
			panel->CreateControls();

			wxAuiPaneInfo paneInfo;
			panel->SetupPaneInfo( paneInfo );
			paneInfo.CloseButton( true );
			paneInfo.Caption( panelTitle ).Name( panelTitle );
			paneInfo.Dockable().Show();
			paneInfo.DestroyOnClose();

			auiManager->AddPane( panel, paneInfo );
			auiManager->Update();
		}
	}

	return true;
}

// ChiCheFrame.cpp
