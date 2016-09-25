// ChiCheSound.cpp

#include "ChiChe.h"

using namespace ChiChe;

//=====================================================================================
Sound::Sound( void )
{
	setup = false;
	enabled = false;
	audioDeviceID = 0;
	maxEffects = 2;
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
	{
		wxString error = SDL_GetError();
		wxMessageBox( "SDL_Init: " + error, "Error", wxCENTRE | wxICON_ERROR );
		return false;
	}
	
	int numAudioDrivers = SDL_GetNumAudioDrivers();
	for( int i = 0; i < numAudioDrivers; i++ )
	{
		const char* audioDriverName = SDL_GetAudioDriver(i);
		audioDriverName = nullptr;
	}
	
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

	wxString soundsDir = "../Share/ChiChe/Sounds";
	wxString snapDir;
	if( wxGetEnv( "SNAP", &snapDir ) )
		soundsDir = snapDir + "/share/ChiChe/Sounds";
	
	wxDir dir( soundsDir );
	if( !dir.IsOpened() )
	{
		soundsDir = "./Sounds";
		if( !dir.Open( soundsDir ) )
		{
			wxMessageBox( "Failed to locate sounds resource directory.", "Error", wxCENTRE | wxICON_ERROR );
			return false;
		}
	}

	wxString waveFile;
	bool found = dir.GetFirst( &waveFile );
	while( found )
	{
		waveFile = soundsDir + "/" + waveFile;
		if( !LoadWave( waveFile ) )
		{
			wxMessageBox( "Failed to load wave file: " + waveFile, "Error", wxCENTRE | wxICON_ERROR );
			return false;
		}
		
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
	Sint16* audioBuffer = ( Sint16* )stream;
	int audioBufLen = streamLen / sizeof( Sint16 );

	Sint32* mixBuffer = new Sint32[ audioBufLen ];
	int mixBufLen = audioBufLen;

	SDL_memset( mixBuffer, 0, sizeof( Sint32 ) * mixBufLen );

	EffectList::iterator iter = effectList.begin();
	while( iter != effectList.end() )
	{
		Effect& effect = *iter;
		Wave* wave = effect.wave;
		Sint16* sampleBuffer = ( Sint16* )&wave->buffer[ effect.offset ];
		int sampleBufLen = ( wave->bufLen - effect.offset ) / sizeof( Sint16 );

		int i = 0;
		while( i < mixBufLen && i < sampleBufLen )
		{
			Sint32 sample = ( Sint32 )sampleBuffer[i];
			mixBuffer[i] += sample;
			i++;
		}

		effect.offset += i * sizeof( Sint16 );
		if( effect.offset < wave->bufLen )
			iter++;
		else
		{
			EffectList::iterator eraseIter = iter;
			iter++;
			effectList.erase( eraseIter );
		}
	}

	for( int i = 0; i < mixBufLen; i++ )
	{
		Sint32 sample = mixBuffer[i];
		if( sample > SHRT_MAX )
			sample = SHRT_MAX;
		if( sample < SHRT_MIN )
			sample = SHRT_MIN;
		audioBuffer[i] = ( Sint16 )sample;
	}

	delete[] mixBuffer;
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
	
	if( wave->waveSpec.format != audioSpec.format ||
		wave->waveSpec.channels != audioSpec.channels ||
		wave->waveSpec.freq != audioSpec.freq )
	{
		return false;
	}

	Effect effect;
	effect.wave = wave;
	effect.offset = 0;

	SDL_LockAudioDevice( audioDeviceID );
	effectList.push_back( effect );
	while( ( signed )effectList.size() > maxEffects )
		effectList.pop_front();
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
