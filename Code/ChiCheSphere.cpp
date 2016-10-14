// ChiCheSphere.cpp

#include "ChiCheSphere.h"
#include <wx/glcanvas.h>

using namespace ChiChe;

// TODO: Might be fun to texture the spheres.

//=====================================================================================
/*static*/ void Sphere::Render( const c3ga::vectorE3GA& location, double radius, int subDivisionCount /*= 2*/ )
{
	double gr = ( 1.0 + sqrt( 5.0 ) ) / 2.0;

	c3ga::vectorE3GA vertex[12];

	vertex[0].set( c3ga::vectorE3GA::coord_e1_e2_e3, -1.0, 0.0, gr );
	vertex[1].set( c3ga::vectorE3GA::coord_e1_e2_e3, 1.0, 0.0, gr );
	vertex[2].set( c3ga::vectorE3GA::coord_e1_e2_e3, -1.0, 0.0, -gr );
	vertex[3].set( c3ga::vectorE3GA::coord_e1_e2_e3, 1.0, 0.0, -gr );

	vertex[4].set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, gr, 1.0 );
	vertex[5].set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, gr, -1.0 );
	vertex[6].set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, -gr, 1.0 );
	vertex[7].set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.0, -gr, -1.0 );

	vertex[8].set( c3ga::vectorE3GA::coord_e1_e2_e3, gr, 1.0, 0.0 );
	vertex[9].set( c3ga::vectorE3GA::coord_e1_e2_e3, -gr, 1.0, 0.0 );
	vertex[10].set( c3ga::vectorE3GA::coord_e1_e2_e3, gr, -1.0, 0.0 );
	vertex[11].set( c3ga::vectorE3GA::coord_e1_e2_e3, -gr, -1.0, 0.0 );

	for( int index = 0; index < 12; index++ )
		vertex[ index ] = c3ga::unit( vertex[ index ] );

	c3ga::mv dilationVersor = ( c3ga::no - 0.5 * radius * c3ga::ni ) * ( c3ga::no - 0.5 * c3ga::ni );
	c3ga::mv translationVersor = 1.0 - 0.5 * location * c3ga::ni;
	c3ga::mv versor = translationVersor * dilationVersor;
	c3ga::evenVersor evenVersor;
	evenVersor.set( versor );

	int face[20][3] =
	{
		{ 1, 4, 0 }, { 4, 9, 0 }, { 4, 5, 9 }, { 8, 5, 4 }, { 1, 8, 4 },
		{ 1, 10, 8 }, { 10, 3, 8 }, { 8, 3, 5 }, { 3, 2, 5 }, { 3, 7, 2 },
		{ 3, 10, 7 }, { 10, 6, 7 }, { 6, 11, 7 }, { 6, 0, 11 }, { 6, 1, 0 },
		{ 10, 1, 6 }, { 11, 0, 9 }, { 2, 11, 9 }, { 5, 2, 9 }, { 11, 2, 7 },
	};

	glBegin( GL_TRIANGLES );
	for( int faceIndex = 0; faceIndex < 20; faceIndex++ )
	{
		int* vertexIndex = face[ faceIndex ];
		RenderSphereTriangle( vertex[ vertexIndex[0] ], vertex[ vertexIndex[1] ], vertex[ vertexIndex[2] ], evenVersor, subDivisionCount );
	}
	glEnd();
}

//=====================================================================================
/*static*/ void Sphere::RenderSphereTriangle(
					const c3ga::vectorE3GA& point0,
					const c3ga::vectorE3GA& point1,
					const c3ga::vectorE3GA& point2,
					const c3ga::evenVersor& evenVersor, int subDivisionCount )
{
	if( subDivisionCount == 0 )
	{
		c3ga::vectorE3GA normal0, normal1, normal2;

		normal0 = point0;
		normal1 = point1;
		normal2 = point2;

		c3ga::normalizedPoint cp0 = c3ga::normalizedPoint( c3ga::normalizedPoint::coord_e1_e2_e3_ni, point0.get_e1(), point0.get_e2(), point0.get_e3(), c3ga::norm2( point0 ) );
		c3ga::normalizedPoint cp1 = c3ga::normalizedPoint( c3ga::normalizedPoint::coord_e1_e2_e3_ni, point1.get_e1(), point1.get_e2(), point1.get_e3(), c3ga::norm2( point1 ) );
		c3ga::normalizedPoint cp2 = c3ga::normalizedPoint( c3ga::normalizedPoint::coord_e1_e2_e3_ni, point2.get_e1(), point2.get_e2(), point2.get_e3(), c3ga::norm2( point2 ) );
		
		// Each of these dual spheres has a radius of zero, so they're really points in space.
		// Notice that while the given versor is NOT a unit-versor, I am applying it here as
		// such, because doing so gives us the normalized dual sphere in the end.
		c3ga::dualSphere ds0 = c3ga::applyUnitVersor( evenVersor, cp0 );
		c3ga::dualSphere ds1 = c3ga::applyUnitVersor( evenVersor, cp1 );
		c3ga::dualSphere ds2 = c3ga::applyUnitVersor( evenVersor, cp2 );

		glNormal3d( normal0.get_e1(), normal0.get_e2(), normal0.get_e3() );
		glVertex3d( ds0.get_e1(), ds0.get_e2(), ds0.get_e3() );

		glNormal3d( normal1.get_e1(), normal1.get_e2(), normal1.get_e3() );
		glVertex3d( ds1.get_e1(), ds1.get_e2(), ds1.get_e3() );

		glNormal3d( normal2.get_e1(), normal2.get_e2(), normal2.get_e3() );
		glVertex3d( ds2.get_e1(), ds2.get_e2(), ds2.get_e3() );
	}
	else
	{
		c3ga::vectorE3GA point01, point12, point20;

		point01 = c3ga::unit( c3ga::add( c3ga::gp( point0, 0.5 ), c3ga::gp( point1, 0.5 ) ) );
		point12 = c3ga::unit( c3ga::add( c3ga::gp( point1, 0.5 ), c3ga::gp( point2, 0.5 ) ) );
		point20 = c3ga::unit( c3ga::add( c3ga::gp( point2, 0.5 ), c3ga::gp( point0, 0.5 ) ) );

		subDivisionCount--;
		RenderSphereTriangle( point0, point01, point20, evenVersor, subDivisionCount );
		RenderSphereTriangle( point1, point12, point01, evenVersor, subDivisionCount );
		RenderSphereTriangle( point2, point20, point12, evenVersor, subDivisionCount );
		RenderSphereTriangle( point01, point12, point20, evenVersor, subDivisionCount );
	}
}

// ChiCheSphere.cpp