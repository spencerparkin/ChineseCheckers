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

		Client* GetClient( void );
		Server* GetServer( void );

	private:

		Frame* frame;
		Client* client;
		Server* server;
	};

	//=====================================================================================
	DECLARE_APP( App );
}

// ChiCheApp.h