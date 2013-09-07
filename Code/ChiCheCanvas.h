// ChiCheCanvas.h

namespace ChiChe
{
	//=====================================================================================
	class Canvas : public wxGLCanvas
	{
	public:

		static int attributeList[];

		Canvas( wxWindow* parent );
		virtual ~Canvas( void );

		void BindContext( void );

	private:

		//=====================================================================================
		class Camera
		{
		public:

			Camera( void );
			~Camera( void );

			void SetAspectRatio( double aspectRatio );
			double GetAspectRatio( void );

			void LoadViewingMatrices( void );

		private:

			c3ga::vectorE3GA eye;
			c3ga::vectorE3GA focus;
			c3ga::vectorE3GA up;

			double foviAngle;
			double aspectRatio;
		};

		void PreRender( GLenum renderMode );
		void PostRender( GLenum renderMode );

		void RenderAxes( void );

		void OnPaint( wxPaintEvent& event );
		void OnSize( wxSizeEvent& event );

		wxGLContext* context;
		Camera camera;
	};
}

// ChiCheCanvas.h