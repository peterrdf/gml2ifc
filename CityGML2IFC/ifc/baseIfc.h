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


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "IFCEngine.h"


typedef struct TRANSFORMATIONMATRIXSTRUCT {
	double			_11;
	double			_12;
	double			_13;
	double			_21;
	double			_22;
	double			_23;
	double			_31;
	double			_32;
	double			_33;
	double			_41;
	double			_42;
	double			_43;
}	transformationMatrixStruct;

typedef struct POINT3DSTRUCT {
	double			x;
	double			y;
	double			z;
    int_t             ifcCartesianPointInstance;
}	point3DStruct;


void	identityMatrix(transformationMatrixStruct * pMatrix);
void	identityPoint(point3DStruct * pPoint);
int_t		* getTimeStamp();


//
//
//	GUID
//
//


static const char *cConversionTable64 =
 "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_$";

char * CreateCompressedGuidString();
char * getString64FromGuid(const GUID *pGuid, char * buf);
BOOL cv_to_64(const unsigned long number, char *code, int_t len);


int_t	    createEmptyIfcFile(wchar_t * ifcSchemaName, bool objectsWillBeAdded, char * lengthUnitConversion);
bool    saveIfcFile(wchar_t * ifcFileName);
bool    saveIfcFileAsXml(wchar_t * ifcFileName);


//
//
//		Application, Organization, Person (OwnerHistory, PersonAndOrganization)
//
//


int_t		getApplicationInstance();
int_t		getOrganizationInstance();
int_t		getOwnerHistoryInstance();
int_t		getPersonAndOrganizationInstance();
int_t		getPersonInstance();


//
//
//		CartesianPoint, Direction, LocalPlacement (Axis2Placement)
//
//


int_t		buildAxis2Placement3DInstance(transformationMatrixStruct * pMatrix);
int_t		buildCartesianPointInstance(point3DStruct * pPoint);
int_t		buildDirectionInstance(point3DStruct * pPoint);
int_t		buildLocalPlacementInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo);


//
//
//		ConversionBasedUnit, DimensionalExponents, MeasureWithUnit, SIUnit, UnitAssignment
//
//


int_t		getConversionBasedUnitInstance();
int_t		getDimensionalExponentsInstance();
int_t		buildMeasureWithUnitInstance();
int_t		buildSIUnitInstance(char * UnitType, char * Prefix, char * Name);
int_t		getUnitAssignmentInstance(char * lengthUnitConversion);

	
//
//
//		RelAggregates, RelContainedInSpatialStructure
//
//


int_t		buildRelAggregatesInstance(char * name, char * description, int_t ifcRelatingObjectInstance, int_t ifcRelatedObjectInstance);
int_t		buildRelContainedInSpatialStructureInstance(char * name, char * description, int_t ifcRelatingStructureInstance, int_t ** aggrRelatedElements);


//
//
//		Building, BuildingStorey, Project, Site
//
//


int_t		buildBuildingInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcBuildingInstancePlacement);
int_t		buildBuildingStoreyInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcBuildingStoreyInstancePlacement);
int_t		getProjectInstance(char * lengthUnitConversion);
int_t		buildSiteInstance(transformationMatrixStruct * pMatrix, int_t ifcPlacementRelativeTo, int_t * ifcSiteInstancePlacement);


//
//
//		WorldCoordinateSystem, GeometricRepresentationContext
//
//


int_t     getWorldCoordinateSystemInstance();
int_t     getGeometricRepresentationContextInstance();


//
//
//		PostalAddress
//
//


int_t		buildPostalAddress();
