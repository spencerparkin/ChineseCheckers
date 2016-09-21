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
	
	private:
	
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
		
		SDL_AudioSpec audioSpec;
		SDL_AudioDeviceID audioDeviceID;
		bool setup;
	};
}

// ChiCheSound.h