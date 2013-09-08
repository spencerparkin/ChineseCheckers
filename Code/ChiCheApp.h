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

	private:

		Frame* frame;
		Client* client;
		Server* server;
	};

	//=====================================================================================
	DECLARE_APP( App );
}

// ChiCheApp.h