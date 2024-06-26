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


#include "baseIfc.h"


typedef struct POINT2DLISTSTRUCT {
	double				x;
	double				y;
    POINT2DLISTSTRUCT	* next;
}	point2DListStruct;


int_t		createIfcWall(char * pWallName, transformationMatrixStruct matrix);
int_t		createIfcWall(char * pWallName, double xOffset, double yOffset, double zOffset);
int_t		createIfcWallStandardCase(char * pWallName, double xOffset, double yOffset, double zOffset);
int_t		createIfcOpeningElement(char * pOpeningElementName, double xOffset, double yOffset, double zOffset, bool representation);
int_t		createIfcOpeningElement(char * pOpeningElementName,
                                double xRefDirection, double yRefDirection, double zRefDirection,
                                double xAxis, double yAxis, double zAxis,
                                double xOffset, double yOffset, double zOffset,
                                bool representation);
int_t		createIfcDoor(char * pDoorName, double xOffset, double yOffset, double zOffset, bool insideOpening, double overallHeight, double overallWidth);
int_t		createIfcDoor(char * pDoorName,
                      double xRefDirection, double yRefDirection, double zRefDirection,
                      double xAxis, double yAxis, double zAxis,
                      double xOffset, double yOffset, double zOffset,
                      bool insideOpening,
                      double overallHeight,
                      double overallWidth);
int_t		createIfcWindow(char * pWindowName, double xOffset, double yOffset, double zOffset, bool insideOpening, double overallHeight, double overallWidth);
int_t		createIfcWindow(char * pWindowName,
                        double xRefDirection, double yRefDirection, double zRefDirection,
                        double xAxis, double yAxis, double zAxis,
                        double xOffset, double yOffset, double zOffset,
                        bool insideOpening,
                        double overallHeight,
                        double overallWidth);
int_t		createIfcSpace(char * pSpaceName, transformationMatrixStruct matrix);
int_t		createIfcRoof(char * pRoofName, transformationMatrixStruct matrix);
int_t		createIfcSlab(char * pSlabName, transformationMatrixStruct matrix);


//
//
//		ProductDefinitionShape
//
//


int_t		buildProductDefinitionShapeInstance();


//
//
//		IfcWall, IfcWallStandardCase, IfcOpeningElement, IfcWindow, IfcSpace, IfcFloor, IfcSlab
//
//


int_t		buildWallInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcWallInstancePlacement, char * pWallName);
int_t		buildWallStandardCaseInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcWallInstancePlacement, char * pWallName);
int_t		buildOpeningElementInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcOpeningElementInstancePlacement, char * pOpeningElementName, bool representation);
int_t		buildDoorInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcDoorInstancePlacement, char * pDoorName, double overallHeight, double overallWidth);
int_t		buildWindowInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcWindowInstancePlacement, char * pWindowName, double overallHeight, double overallWidth);
int_t		buildSpaceInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcSpaceInstancePlacement, char * pSpaceName);
int_t		buildRoofInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcRoofInstancePlacement, char * pRoofName);
int_t		buildSlabInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcSlabInstancePlacement, char * pSlabName);


//
//
//		RelVoidsElement, RelFillsElement
//
//


int_t		buildRelVoidsElementInstance(int_t ifcBuildingElementInstance, int_t ifcOpeningElementInstance);
int_t     buildRelFillsElementInstance(int_t ifcOpeningElementInstance, int_t ifcBuildingElementInstance);


//
//
//      RelAssociatesMaterial, MaterialLayerSetUsage, MaterialLayerSet, MaterialLayer
//
//


int_t		buildRelAssociatesMaterial(int_t ifcBuildingElementInstance, double thickness);
int_t     buildMaterialLayerSetUsage(double thickness);
int_t     buildMaterialLayerSet(double thickness);
int_t     buildMaterialLayer(double thickness);
int_t     buildMaterial();


//
//
//		RelSpaceBoundary
//
//


int_t		buildRelSpaceBoundaryInstance(int_t ifcRelatingSpaceInstance, int_t ifcRelatedBuildingElementInstance, char * pSpaceBoundaryName, char * pSpaceBoundaryDescription, transformationMatrixStruct * pMatrix, point2DListStruct * pPoints);
int_t		buildRelSpaceBoundary1stLevelInstance(int_t ifcRelatingSpaceInstance, int_t ifcRelatedBuildingElementInstance, char * pSpaceBoundaryName, char * pSpaceBoundaryDescription, transformationMatrixStruct * pMatrix, point2DListStruct * pPoints, int_t parentBoundary);
int_t		buildRelSpaceBoundary2ndLevelInstance(int_t ifcRelatingSpaceInstance, int_t ifcRelatedBuildingElementInstance, char * pSpaceBoundaryName, char * pSpaceBoundaryDescription, transformationMatrixStruct * pMatrix, point2DListStruct * pPoints, int_t parentBoundary, int_t correspondingBoundary);
int_t		buildConnectionSurfaceGeometryInstance(transformationMatrixStruct * pMatrix, point2DListStruct * pPoints);
int_t		buildCurveBoundedPlaneInstance(transformationMatrixStruct * pMatrix, point2DListStruct * pPoints);
int_t		buildPlaneInstance(transformationMatrixStruct * pMatrix);
int_t		build2DCompositeCurveInstance(point2DListStruct * pPoints);
int_t		buildCompositeCurveSegmentInstance(point2DListStruct * pPoints);
int_t		buildPolylineInstance(point2DListStruct * pPoints);
int_t		buildCartesianPointInstance(point2DListStruct * pPoint2D);