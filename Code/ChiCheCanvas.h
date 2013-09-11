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

		double FrameRate( void );

	private:

		//=====================================================================================
		class Camera
		{
		public:

			Camera( void );
			~Camera( void );

			void SetAspectRatio( double aspectRatio );
			double GetAspectRatio( void );

			void LoadViewingMatrices( wxPoint* pickingPoint = 0 );

			void Zoom( double zoomPercentage, double minimumFocalLength, double maximumFocalLength );
			double FocalLength( void );

			void CalcPanFrame( c3ga::vectorE3GA& xAxis, c3ga::vectorE3GA& yAxis, c3ga::vectorE3GA& zAxis );
			void Move( const c3ga::vectorE3GA& delta, bool maintainFocalLength = true );

			const c3ga::vectorE3GA& GetEye( void );
			void SetEye( const c3ga::vectorE3GA& eye );

			const c3ga::vectorE3GA& GetFocus( void );
			void SetFocus( const c3ga::vectorE3GA& focus );

		private:

			c3ga::vectorE3GA eye;
			c3ga::vectorE3GA focus;
			c3ga::vectorE3GA up;

			double foviAngle;
			double aspectRatio;
		};

		void PreRender( GLenum renderMode );
		void PostRender( GLenum renderMode );

		void OnPaint( wxPaintEvent& event );
		void OnSize( wxSizeEvent& event );
		void OnMouseWheel( wxMouseEvent& event );
		void OnMouseLeftDown( wxMouseEvent& event );
		void OnMouseRightDown( wxMouseEvent& event );
		void OnMouseMotion( wxMouseEvent& event );

		wxGLContext* context;
		Camera camera;
		wxPoint mousePos;
		unsigned int* hitBuffer;
		int hitBufferSize;
		wxLongLong lastFrameTime;
		double frameRate;
	};
}

// ChiCheCanvas.h