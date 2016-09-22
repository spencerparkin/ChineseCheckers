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

		static void AudioCallback( void* userdata, Uint8* stream, int length );
	
		void PullForAudio( Uint8* stream, int length );

		class Wave
		{
		public:
			Wave( void );
			virtual ~Wave( void );
		
			bool Load( const wxString& waveFile );
			bool Unload( void );
		
			wxString waveName;
			SDL_AudioSpec waveSpec;
			Uint32 waveLength;
			Uint8* waveBuffer;
		};
		
		Wave* FindWave( const wxString& waveName );
		
		typedef std::list< Wave* > WaveList;
		WaveList waveList;

		struct Effect
		{
			Wave* wave;
			Uint32 waveOffset;
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