// ChiCheSound.h

namespace ChiChe
{
	//=====================================================================================
	class Sound
	{
	public:
	
		Sound( void );
		virtual ~Sound( void );
	
		bool Setup( void );
		bool Shutdown( void );
	
		bool ClearWaves( void );
		bool LoadWave( const wxString& waveFile );
		bool PlayWave( const wxString& waveName );
	
		bool Enable( bool enable );
		bool IsEnabled( void ) { return enabled; }
		bool IsSetup( void ) { return setup; }

	private:

		static void AudioCallback( void* userdata, Uint8* stream, int streamLen );
	
		void PullForAudio( Uint8* stream, int streamLen );

		class Wave
		{
		public:
			Wave( void );
			virtual ~Wave( void );
		
			bool Load( const wxString& waveFile );
			bool Unload( void );
		
			wxString name;
			SDL_AudioSpec waveSpec;
			Uint32 bufLen;
			Uint8* buffer;
		};
		
		Wave* FindWave( const wxString& waveName );
		
		typedef std::list< Wave* > WaveList;
		WaveList waveList;

		struct Effect
		{
			Wave* wave;
			Uint32 offset;
		};

		typedef std::list< Effect > EffectList;
		EffectList effectQueue;
		
		SDL_AudioSpec audioSpec;
		SDL_AudioDeviceID audioDeviceID;
		bool setup;
		bool enabled;
	};
}

// ChiCheSound.h