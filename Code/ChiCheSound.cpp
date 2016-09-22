// ChiCheSound.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Sound::Sound( void )
{
	setup = false;
	enabled = false;
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
	
	int numAudioDevices = SDL_GetNumAudioDevices(0);
	for( int i = 0; i < numAudioDevices; i++ )
	{
		const char* audioDeviceName = SDL_GetAudioDeviceName( i, 0 );
		audioDeviceName = nullptr;
	}

	SDL_AudioSpec desiredAudioSpec;
	SDL_memset( &desiredAudioSpec, 0, sizeof( SDL_AudioSpec ) );
	desiredAudioSpec.freq = 2 * 48000;
	desiredAudioSpec.format = AUDIO_S16;
	desiredAudioSpec.channels = 1;
	desiredAudioSpec.samples = 4096;
	desiredAudioSpec.callback = AudioCallback;
	desiredAudioSpec.userdata = this;
	
	audioDeviceID = SDL_OpenAudioDevice( NULL, 0, &desiredAudioSpec, &audioSpec, 0 );
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
	
	Enable( false );

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
bool Sound::Enable( bool enable )
{
	if( !setup )
		return false;

	if( this->enabled != enable )
	{
		this->enabled = enable;

		SDL_PauseAudioDevice( audioDeviceID, ( enabled ? 0 : 1 ) );
	}

	return true;
}

//=====================================================================================
/*static*/ void Sound::AudioCallback( void* userdata, Uint8* stream, int length )
{
	Sound* sound = ( Sound* )userdata;
	sound->PullForAudio( stream, length );
}

//=====================================================================================
void Sound::PullForAudio( Uint8* stream, int length )
{
	if( effectQueue.size() == 0 )
	{
		SDL_memset( stream, 0, length );
	}
	else
	{
		// None of the effects overlap.  I suppose we could
		// get them to overlap if we mixed the audio.  If we
		// wanted background music, we would need to mix the FX
		// with the music.
		while( effectQueue.size() > 0 )
		{
			EffectList::iterator iter = effectQueue.begin();
			Effect& effect = *iter;

			if( effect.wave->waveSpec.format != audioSpec.format )
			{
				effectQueue.erase( iter );
				continue;
			}

			Uint32 effectSize = effect.wave->waveLength - effect.waveOffset;
			if( effectSize <= ( unsigned )length )
			{
				SDL_memcpy( stream, effect.wave->waveBuffer + effect.waveOffset, effectSize );
				stream += effectSize;
				length -= effectSize;
				effectQueue.erase( iter );
			}
			else
			{
				SDL_memcpy( stream, effect.wave->waveBuffer + effect.waveOffset, length );
				effect.waveOffset += length;
				return;
			}
		}

		SDL_memset( stream, 0, length );
	}
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
	if( !setup || !enabled )
		return false;

	Wave* wave = FindWave( waveName );
	if( !wave )
		return false;
	
	Effect effect;
	effect.wave = wave;
	effect.waveOffset = 0;

	SDL_LockAudioDevice( audioDeviceID );
	effectQueue.push_back( effect );
	SDL_UnlockAudioDevice( audioDeviceID );

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
	
	waveName = fileName.GetName();
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