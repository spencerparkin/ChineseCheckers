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
	};

	//=====================================================================================
	DECLARE_APP( App );
}

// ChiCheApp.h