// ChiCheApp.h

namespace ChiChe
{
	//=====================================================================================
	class App : public wxApp
	{
	public:

		App( void );
		virtual ~App( void );

		virtual bool OnInit( void ) override;
		virtual int OnExit( void ) override;

		void SetClient( Client* client );
		Client* GetClient( void );

		void SetServer( Server* server );
		Server* GetServer( void );

		Frame* GetFrame( void );

		Sound* GetSound( void );
	
		wxString soundEffect;

	private:

		Frame* frame;
		Client* client;
		Server* server;
		Sound* sound;
	};
}

//=====================================================================================
DECLARE_APP( ChiChe::App );

// ChiCheApp.h
