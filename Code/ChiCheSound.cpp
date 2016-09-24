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
	
	int numAudioDevices = SDL_GetNumAudioDevices(0);
	for( int i = 0; i < numAudioDevices; i++ )
	{
		const char* audioDeviceName = SDL_GetAudioDeviceName( i, 0 );
		audioDeviceName = nullptr;
	}

	SDL_AudioSpec desiredAudioSpec;
	SDL_memset( &desiredAudioSpec, 0, sizeof( SDL_AudioSpec ) );
	desiredAudioSpec.freq = 48000;
	desiredAudioSpec.format = AUDIO_S16;
	desiredAudioSpec.channels = 2;
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
/*static*/ void Sound::AudioCallback( void* userdata, Uint8* stream, int streamLen )
{
	Sound* sound = ( Sound* )userdata;
	sound->PullForAudio( stream, streamLen );
}

//=====================================================================================
void Sound::PullForAudio( Uint8* stream, int streamLen )
{
	// TODO: We need to mix the audio here so that FX happen when they're supposed to.
	if( effectQueue.size() == 0 )
		SDL_memset( stream, 0, streamLen );
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
			Wave* wave = effect.wave;

			if( wave->waveSpec.format != audioSpec.format ||
				wave->waveSpec.channels != audioSpec.channels ||
				wave->waveSpec.freq != audioSpec.freq )
			{
				effectQueue.erase( iter );
				continue;
			}

			Uint32 effectSize = wave->bufLen - effect.offset;
			if( effectSize <= ( unsigned )streamLen )
			{
				SDL_memcpy( stream, wave->buffer + effect.offset, effectSize );
				stream += effectSize;
				streamLen -= effectSize;
				effectQueue.erase( iter );
			}
			else
			{
				SDL_memcpy( stream, wave->buffer + effect.offset, streamLen );
				effect.offset += streamLen;
				return;
			}
		}

		SDL_memset( stream, 0, streamLen );
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
	effect.offset = 0;

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
		if( wave->name == waveName )
			return wave;

		iter++;
	}
	
	return nullptr;
}

//=====================================================================================
Sound::Wave::Wave( void )
{
	bufLen = 0;
	buffer = nullptr;
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
	
	SDL_memset( &waveSpec, 0, sizeof( SDL_AudioSpec ) );
	if( NULL == SDL_LoadWAV( waveFile.c_str(), &waveSpec, &buffer, &bufLen ) )
	{
		wxString error = SDL_GetError();
		wxMessageBox( "SDL_LoadWAV: " + error, "Error", wxCENTRE | wxICON_ERROR );
		return false;
	}
	
	name = fileName.GetName();
	return true;
}

//=====================================================================================
bool Sound::Wave::Unload( void )
{
	if( buffer )
	{
		SDL_FreeWAV( buffer );
		buffer = nullptr;
		bufLen = 0;
	}
	
	return true;
}

// ChiCheSound.h