// ChiCheFrame.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Frame::Frame( void ) : wxFrame( 0, wxID_ANY, "Chinese Checkers", wxDefaultPosition, wxSize( 900, 600 ) ), timer( this, ID_Timer )
{
	wxMenu* gameMenu = new wxMenu();
	wxMenuItem* joinGameMenuItem = new wxMenuItem( gameMenu, ID_JoinGame, wxT( "Join Game" ), wxT( "Join a hosted game on the network." ) );
	wxMenuItem* hostGameMenuItem = new wxMenuItem( gameMenu, ID_HostGame, wxT( "Host Game" ), wxT( "Host a game on the network." ) );
	wxMenuItem* exitMenuItem = new wxMenuItem( gameMenu, ID_Exit, wxT( "Exit" ), wxT( "Exit the program." ) );
	gameMenu->Append( joinGameMenuItem );
	gameMenu->Append( hostGameMenuItem );
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
	statusBar->PushStatusText( wxT( "Welcome!" ) );
	SetStatusBar( statusBar );

	Bind( wxEVT_MENU, &Frame::OnJoinGame, this, ID_JoinGame );
	Bind( wxEVT_MENU, &Frame::OnHostGame, this, ID_HostGame );
	Bind( wxEVT_MENU, &Frame::OnExit, this, ID_Exit );
	Bind( wxEVT_MENU, &Frame::OnAbout, this, ID_About );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_JoinGame );
	Bind( wxEVT_UPDATE_UI, &Frame::OnUpdateMenuItemUI, this, ID_HostGame );
	Bind( wxEVT_TIMER, &Frame::OnTimer, this, ID_Timer );
	Bind( wxEVT_CLOSE_WINDOW, &Frame::OnClose, this );

	canvas = new Canvas( this );

	wxBoxSizer* boxSizer = new wxBoxSizer( wxHORIZONTAL );
	boxSizer->Add( canvas, 1, wxALL | wxGROW, 0 );
	SetSizer( boxSizer );

	timer.Start( 50 );
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
	typeChoices.Add( "Computer" );
	wxSingleChoiceDialog singleChoiceDialog( this, wxT( "Will this be a human or computer client?" ), wxT( "Choose Client Type" ), typeChoices );
	if( wxID_OK != singleChoiceDialog.ShowModal() )
		return;

	Client::Type clientType = Client::HUMAN;
	if( singleChoiceDialog.GetStringSelection() == wxT( "Computer" ) )
		clientType = Client::COMPUTER;

	wxScopedPtr< Client > clientPtr;
	clientPtr.reset( new Client( clientType ) );

	if( !clientPtr->Connect( address ) )
		return;

	client = clientPtr.release();
	wxGetApp().SetClient( client );
}

//=====================================================================================
void Frame::OnExit( wxCommandEvent& event )
{
	Close( true );
}

//=====================================================================================
void Frame::OnClose( wxCloseEvent& event )
{
	Server* server = wxGetApp().GetServer();
	if( server )
	{
		server->Finalize();
		delete server;
		wxGetApp().SetServer(0);
	}

	Client* client = wxGetApp().GetClient();
	if( client )
	{
		delete client;
		wxGetApp().SetClient(0);
	}

	event.Skip();
}

//=====================================================================================
void Frame::OnAbout( wxCommandEvent& event )
{
	wxAboutDialogInfo aboutDialogInfo;
	
	aboutDialogInfo.SetName( wxT( "Chinese Checkers" ) );
	aboutDialogInfo.SetVersion( wxT( "1.0" ) );
	aboutDialogInfo.SetDescription( wxT( "This program is free software and distributed under the MIT license." ) );
	aboutDialogInfo.SetCopyright( wxT( "Copyright (C) 2013 Spencer T. Parkin <spencer.parkin@disney.com>" ) );

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
	}
}

//=====================================================================================
void Frame::OnTimer( wxTimerEvent& event )
{
	Server* server = wxGetApp().GetServer();
	if( server )
	{
		if( !server->Run() )
		{
			server->Finalize();
			delete server;
			wxGetApp().SetServer(0);
		}
	}

	Client* client = wxGetApp().GetClient();
	if( client )
	{
		if( !client->Run() )
		{
			delete client;
			wxGetApp().SetClient(0);
		}
		else
		{
			client->Animate( canvas->FrameRate() );

			// TODO: We probably shouldn't be thrashing OnPaint events when the window is not active.
			canvas->Refresh();
		}
	}
}

//=====================================================================================
wxStatusBar* Frame::GetStatusBar( void )
{
	return statusBar;
}

// ChiCheFrame.cpp