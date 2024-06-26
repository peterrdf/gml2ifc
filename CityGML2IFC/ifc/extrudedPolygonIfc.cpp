////////////////////////////////////////////////////////////////////////
//  Author:  Peter Bonsma
//  Date:    22 September 2010
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


#include "pch.h"
#include "extrudedPolygonIfc.h"

extern  int_t     model;

extern  int_t     * aggrRelatedElements,
                * aggrRepresentations;


void    createIfcExtrudedPolygonShape(polygon2DStruct * pPolygon, double depth)
{
	ASSERT(aggrRepresentations);

    sdaiAppend(aggrRepresentations, sdaiINSTANCE, (void*) buildShapeRepresentationInstance(pPolygon, depth));
}

void    createIfcPolylineShape(double p0x, double p0y, double p1x, double p1y)
{
	ASSERT(aggrRepresentations);

	sdaiAppend(aggrRepresentations, sdaiINSTANCE, (void*) buildShapeRepresentationInstance(p0x, p0y, p1x, p1y));
}


//
//
//		ShapeRepresentation
//
//


int_t		buildShapeRepresentationInstance(polygon2DStruct * pPolygon, double depth)
{
	int_t		ifcShapeRepresentationInstance, * aggrItems;

	ifcShapeRepresentationInstance = sdaiCreateInstanceBN(model, "IFCSHAPEREPRESENTATION");

	aggrItems = sdaiCreateAggrBN(ifcShapeRepresentationInstance, "Items");
	sdaiAppend(aggrItems, sdaiINSTANCE, (void*) buildExtrudedAreaSolidInstance(pPolygon, depth));

	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "SweptSolid");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*) getGeometricRepresentationContextInstance());

	ASSERT(ifcShapeRepresentationInstance);

	return	ifcShapeRepresentationInstance;
}

int_t		buildShapeRepresentationInstance(double p0x, double p0y, double p1x, double p1y)
{
	int_t		ifcShapeRepresentationInstance, * aggrItems;

	ifcShapeRepresentationInstance = sdaiCreateInstanceBN(model, "IFCSHAPEREPRESENTATION");

	aggrItems = sdaiCreateAggrBN(ifcShapeRepresentationInstance, "Items");
	sdaiAppend(aggrItems, sdaiINSTANCE, (void*) buildPolylineInstance(p0x, p0y, p1x, p1y));

	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Axis");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Curve2D");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*) getGeometricRepresentationContextInstance());

	ASSERT(ifcShapeRepresentationInstance);

	return	ifcShapeRepresentationInstance;
}


//
//
//		ArbitraryClosedProfileDef, CartesianPoint(2D), ExtrudedAreaSolid, Polyline
//
//


int_t		buildArbitraryClosedProfileDefInstance(polygon2DStruct * pPolygon)
{
	int_t		ifcArbitraryClosedProfileDefInstance;

	ifcArbitraryClosedProfileDefInstance = sdaiCreateInstanceBN(model, "IFCARBITRARYCLOSEDPROFILEDEF");

	sdaiPutAttrBN(ifcArbitraryClosedProfileDefInstance, "ProfileType", sdaiENUM, "AREA");
	sdaiPutAttrBN(ifcArbitraryClosedProfileDefInstance, "OuterCurve", sdaiINSTANCE, (void*) buildPolylineInstance(pPolygon));

	ASSERT(ifcArbitraryClosedProfileDefInstance);

	return	ifcArbitraryClosedProfileDefInstance;
}

int_t		buildCartesianPointInstance(double x, double y)
{
	int_t		ifcCartesianPointInstance, * aggrCoordinates;

	ifcCartesianPointInstance = sdaiCreateInstanceBN(model, "IFCCARTESIANPOINT");

	aggrCoordinates = sdaiCreateAggrBN(ifcCartesianPointInstance, "Coordinates");
	sdaiAppend(aggrCoordinates, sdaiREAL, &x);
	sdaiAppend(aggrCoordinates, sdaiREAL, &y);

	ASSERT(ifcCartesianPointInstance);

	return	ifcCartesianPointInstance;
}

int_t		buildExtrudedAreaSolidInstance(polygon2DStruct * pPolygon, double depth)
{
	transformationMatrixStruct  matrix;
	int_t		ifcExtrudedAreaSolidInstance;

	identityMatrix(&matrix);

	ifcExtrudedAreaSolidInstance = sdaiCreateInstanceBN(model, "IFCEXTRUDEDAREASOLID");

	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "SweptArea", sdaiINSTANCE, (void*) buildArbitraryClosedProfileDefInstance(pPolygon));
	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "Position", sdaiINSTANCE, (void*) buildAxis2Placement3DInstance(&matrix));
	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "ExtrudedDirection", sdaiINSTANCE, (void*) buildDirectionInstance((point3DStruct *) &matrix._31));
	sdaiPutAttrBN(ifcExtrudedAreaSolidInstance, "Depth", sdaiREAL, (void *) &depth);

	ASSERT(ifcExtrudedAreaSolidInstance);

	return	ifcExtrudedAreaSolidInstance;
}

int_t		buildPolylineInstance(polygon2DStruct * pPolygon)
{
	int_t				ifcPolylineInstance, * aggrPoints;
	vector2DStruct	* pVector = pPolygon->pVector;

	ifcPolylineInstance = sdaiCreateInstanceBN(model, "IFCPOLYLINE");

	aggrPoints = sdaiCreateAggrBN(ifcPolylineInstance, "Points");
    double  x = pVector->pPoint->x,
            y = pVector->pPoint->y;
    while  (pVector) {
	    sdaiAppend(aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(pVector->pPoint->x, pVector->pPoint->y));

        pVector = pVector->next;
    }
	sdaiAppend(aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(x, y));

	ASSERT(ifcPolylineInstance);

	return	ifcPolylineInstance;
}

int_t		buildPolylineInstance(double p0x, double p0y, double p1x, double p1y)
{
	int_t		ifcPolylineInstance, * aggrPoints;

	ifcPolylineInstance = sdaiCreateInstanceBN(model, "IFCPOLYLINE");

	aggrPoints = sdaiCreateAggrBN(ifcPolylineInstance, "Points");
	sdaiAppend(aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(p0x, p0y));
	sdaiAppend(aggrPoints, sdaiINSTANCE, (void*) buildCartesianPointInstance(p1x, p1y));

	ASSERT(ifcPolylineInstance);

	return	ifcPolylineInstance;
}
