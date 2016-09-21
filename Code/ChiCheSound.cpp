// ChiCheSound.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Sound::Sound( void )
{
	setup = false;
	audioDeviceID = 0;
}

//=====================================================================================
/*virtual*/ Sound::~Sound( void )
{
	if( setup )
		Shutdown();
}

//=====================================================================================
bool Sound::Setup( void )
{
	if( 0 != SDL_Init( SDL_INIT_AUDIO ) )
		return false;
	
	wxDir dir( wxGetCwd() + "/Sounds" );
	if( !dir.IsOpened() )
		return false;
	
	wxString waveFile;
	bool found = dir.GetFirst( &waveFile );
	while( found )
	{
		waveFile = wxGetCwd() + "/Sounds/" + waveFile;
		
		( void )LoadWave( waveFile );
		
		found = dir.GetNext( &waveFile );
	}
	
	SDL_AudioSpec desiredAudioSpec;
	SDL_memset( &desiredAudioSpec, 0, sizeof( SDL_AudioSpec ) );
	desiredAudioSpec.freq = 48000;
	desiredAudioSpec.format = AUDIO_F32;
	desiredAudioSpec.channels = 1;
	desiredAudioSpec.samples = 4096;
	desiredAudioSpec.callback = NULL;
	
	audioDeviceID = SDL_OpenAudioDevice( NULL, 0, &desiredAudioSpec, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE );
	if( audioDeviceID == 0 )
	{
		wxString error = SDL_GetError();
		wxMessageBox( "SDL_OpenAudioDevice: " + error, "Error", wxCENTRE | wxICON_ERROR );
		return false;
	}
	
	setup = true;
	return true;
}

//=====================================================================================
bool Sound::Shutdown( void )
{
	if( !setup )
		return false;
	
	ClearWaves();
	
	if( audioDeviceID != 0 )
	{
		SDL_CloseAudioDevice( audioDeviceID );
		audioDeviceID = 0;
	}
	
	SDL_Quit();
	
	setup = false;
	return true;
}

//=====================================================================================
bool Sound::LoadWave( const wxString& waveFile )
{
	Wave* wave = new Wave();
	if( !wave->Load( waveFile ) )
	{
		delete wave;
		return false;
	}
	
	waveList.push_back( wave );
	return true;
}

//=====================================================================================
bool Sound::ClearWaves( void )
{
	while( waveList.size() > 0 )
	{
		WaveList::iterator iter = waveList.begin();
		Wave* wave = *iter;
		delete wave;
		waveList.erase( iter );
	}
	
	return true;
}

//=====================================================================================
bool Sound::PlayWave( const wxString& waveName )
{
	Wave* wave = FindWave( waveName );
	if( !wave )
		return false;
	
	//...go play the wave here...
	return true;
}

//=====================================================================================
Sound::Wave* Sound::FindWave( const wxString& waveName )
{
	WaveList::iterator iter = waveList.begin();
	while( iter != waveList.end() )
	{
		Wave* wave = *iter;
		if( wave->waveName == waveName )
			return wave;
	}
	
	return nullptr;
}

//=====================================================================================
Sound::Wave::Wave( void )
{
	waveLength = 0;
	waveBuffer = nullptr;
}

//=====================================================================================
/*virtual*/ Sound::Wave::~Wave( void )
{
	Unload();
}

//=====================================================================================
bool Sound::Wave::Load( const wxString& waveFile )
{
	wxFileName fileName( waveFile );
	if( !fileName.FileExists() )
		return false;
	
	if( NULL == SDL_LoadWAV( waveFile.c_str(), &waveSpec, &waveBuffer, &waveLength ) )
	{
		wxString error = SDL_GetError();
		wxMessageBox( "SDL_LoadWAVE: " + error, "Error", wxCENTRE | wxICON_ERROR );
		return false;
	}
	
	return true;
}

//=====================================================================================
bool Sound::Wave::Unload( void )
{
	if( waveBuffer )
	{
		SDL_FreeWAV( waveBuffer );
		waveBuffer = nullptr;
		waveLength = 0;
	}
	
	return true;
}

// ChiCheSound.h