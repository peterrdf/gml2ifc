////////////////////////////////////////////////////////////////////////
//  Author:  Peter Bonsma
//  Date:    31 July 2010
//  Project: IFC Engine Series (example using DLL)
//
//  This code may be used and edited freely,
//  also for commercial projects in open and closed source software
//
//  In case of use of the DLL:
//  be aware of license fee for use of this DLL when used commercially
//  more info for commercial use:	peter.bonsma@tno.nl
//
//  more info for using the IFC Engine DLL in other languages
//	and creation of specific code examples:
//									pim.vandenhelm@tno.nl
//								    peter.bonsma@rdf.bg
////////////////////////////////////////////////////////////////////////


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "boundingBoxIfc.h"


typedef struct POINT2DSTRUCT {
	double			x;
	double			y;
}	point2DStruct;

typedef struct VECTOR2DSTRUCT {
	POINT2DSTRUCT   * pPoint;
	VECTOR2DSTRUCT  * next;
}	vector2DStruct;

typedef struct POLYGON2DSTRUCT {
	VECTOR2DSTRUCT  * pVector;
	POLYGON2DSTRUCT * next;
}	polygon2DStruct;



void    createIfcExtrudedPolygonShape(polygon2DStruct * pPolygon, double depth);
void    createIfcPolylineShape(double p0x, double p0y, double p1x, double p1y);


//
//
//		ShapeRepresentation
//
//


int_t		buildShapeRepresentationInstance(polygon2DStruct * pPolygon, double depth);
int_t		buildShapeRepresentationInstance(double p0x, double p0y, double p1x, double p1y);


//
//
//		ArbitraryClosedProfileDef, CartesianPoint(2D), ExtrudedAreaSolid, Polyline
//
//


int_t		buildArbitraryClosedProfileDefInstance(polygon2DStruct * pPolygon);
int_t		buildCartesianPointInstance(double x, double y);
int_t		buildExtrudedAreaSolidInstance(polygon2DStruct * pPolygon, double depth);
int_t		buildPolylineInstance(polygon2DStruct * pPolygon);
int_t		buildPolylineInstance(double p0x, double p0y, double p1x, double p1y);
