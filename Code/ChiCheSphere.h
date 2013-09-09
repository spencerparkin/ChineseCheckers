// ChiCheSphere.h

namespace ChiChe
{
	//=====================================================================================
	class Sphere
	{
	public:

		static void Render( const c3ga::vectorE3GA& location, double radius, int subDivisionCount = 2 );

	private:

		static void RenderSphereTriangle(
							const c3ga::vectorE3GA& point0,
							const c3ga::vectorE3GA& point1,
							const c3ga::vectorE3GA& point2,
							const c3ga::evenVersor& evenVersor, int subDivisionCount );
	};
}

// ChiCheSphere.h