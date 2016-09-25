// ChiCheFrame.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Frame::Frame( void ) : wxFrame( 0, wxID_ANY, "Chinese Checkers", wxDefaultPosition, wxSize( 900, 600 ) ), timer( this, ID_Timer )
{
	wxMenu* gameMenu = new wxMenu();
	wxMenuItem* joinGameMenuItem = new wxMenuItem( gameMenu, ID_JoinGame, wxT( "Join Game" ), wxT( "Join a hosted game on the network." ) );
	wxMenuItem* hostGameMenuItem = new wxMenuItem( gameMenu, ID_HostGame, wxT( "Host Game" ), wxT( "Host a game on the network." ) );
	wxMenuItem* leaveGameMenuItem = new wxMenuItem( gameMenu, ID_LeaveGame, wxT( "Leave Game" ), wxT( "Disconnect from a joined game." ) );
	wxMenuItem* killGameMenuItem = new wxMenuItem( gameMenu, ID_KillGame, wxT( "Kill Game" ), wxT( "Discontinue a hosted game." ) );
	wxMenuItem* toggleSoundMenuItem = new wxMenuItem( gameMenu, ID_ToggleSound, wxT( "Sound" ), wxT( "Toggle the playing of sound FX." ), wxITEM_CHECK );
	wxMenuItem* effectMenuItem = new wxMenuItem( gameMenu, ID_Effect, wxT( "Effect" ), wxT( "Choose your sound effect." ) );
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
	gameMenu->Append( exitMenuItem );

	wxMenu* helpMenu = new wxMenu();
	wxMenuItem* aboutMenuItem = new wxMenuItem( helpMenu, ID_About, wxT( "About" ), wxT( "Popup a dialog giving information about this program." ) );
	helpMenu->Append( aboutMenuItem );

	menuBar = new wxMenuBar();
	menuBar->Append( gameMenu, wxT( "Game" ) );
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
	Bind( wxEVT_MENU, &Frame::OnExit, this, ID_Exit );
	Bind( wxEVT_MENU, &Frame::OnAbout, this, ID_About );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_JoinGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_HostGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_LeaveGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_KillGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_ToggleSound );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_Effect );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_DoinkEffect );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_FartEffect );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_HiyawEffect );
	Bind( wxEVT_TIMER, &Frame::OnTimer, this, ID_Timer );
	Bind( wxEVT_CLOSE_WINDOW, &Frame::OnClose, this );
	Bind( wxEVT_ACTIVATE, &Frame::OnActivate, this );

	canvas = new Canvas( this );

	wxBoxSizer* boxSizer = new wxBoxSizer( wxHORIZONTAL );
	boxSizer->Add( canvas, 1, wxALL | wxGROW, 0 );
	SetSizer( boxSizer );

	timer.Start( 50 );
	continuousRefresh = true;
}

//=====================================================================================
/*virtual*/ Frame::~Frame( void )
{
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
	typeChoices.Add( "Computer (Level 1)" );
	typeChoices.Add( "Computer (Level 2)" );
	typeChoices.Add( "Computer (Level 3)" );
	wxSingleChoiceDialog singleChoiceDialog( this, wxT( "Will this be a human or computer client?" ), wxT( "Choose Client Type" ), typeChoices );
	if( wxID_OK != singleChoiceDialog.ShowModal() )
		return;

	Client::Type clientType = Client::HUMAN;
	wxString selection = singleChoiceDialog.GetStringSelection();
	if( selection == wxT( "Computer (Level 1)" ) )
		clientType = Client::COMPUTER_LEVEL_1;
	else if( selection == wxT( "Computer (Level 2)" ) )
		clientType = Client::COMPUTER_LEVEL_2;
	else if( selection == wxT( "Computer (Level 3)" ) )
		clientType = Client::COMPUTER_LEVEL_3;

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
	aboutDialogInfo.SetDescription( wxT( "This program is free software and distributed under the MIT license.  The AI in this program is terrible; I'm working to improve it." ) );
	aboutDialogInfo.SetCopyright( wxT( "Copyright (C) 2013 Spencer T. Parkin <spencer.parkin@disney.com>" ) );
	aboutDialogInfo.SetWebSite( wxT( "http://spencerparkin.github.io/ChineseCheckers/" ) );

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
			event.Enable( wxGetApp().GetSound()->IsSetup() );
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
			client->Animate( canvas->FrameRate() );
			if( continuousRefresh )
				canvas->Refresh();
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

// ChiCheFrame.cpp
