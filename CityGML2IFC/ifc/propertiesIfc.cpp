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
#include "baseIfc.h"
#include "propertiesIfc.h"

extern  int_t     model;


//
//
//		PropertySet, PropertySingleValue
//
//


int_t		buildPropertySet(char * name, int_t ** aggrHasProperties)
{
	int_t		ifcPropertySetInstance;

	ifcPropertySetInstance = sdaiCreateInstanceBN(model, "IFCPROPERTYSET");

	sdaiPutAttrBN(ifcPropertySetInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcPropertySetInstance, "OwnerHistory", sdaiINSTANCE, (void*) getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcPropertySetInstance, "Name", sdaiSTRING, name);

	(* aggrHasProperties) = sdaiCreateAggrBN(ifcPropertySetInstance, "HasProperties");

	ASSERT(ifcPropertySetInstance);

	return	ifcPropertySetInstance;
}

int_t		buildPropertySingleValue(char * name, char * description, bool nominalValue)
{
	return	buildPropertySingleValue(name, description, nominalValue, "IFCBOOLEAN");
}

int_t		buildPropertySingleValue(char * name, char * description, bool nominalValue, char * typePath)
{
	int_t		ifcPropertySingleValueInstance;
    void    * nominalValueADB;
    char    bTrue[2] = "T", bFalse[2] = "F"; 

	ifcPropertySingleValueInstance = sdaiCreateInstanceBN(model, "IFCPROPERTYSINGLEVALUE");

	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Description", sdaiSTRING, description);

    if  (nominalValue) {
	    nominalValueADB = sdaiCreateADB(sdaiENUM, bTrue);
    } else {
	    nominalValueADB = sdaiCreateADB(sdaiENUM, bFalse);
    }
	sdaiPutADBTypePath(nominalValueADB, 1, typePath); 
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "NominalValue", sdaiADB, (void*) nominalValueADB);

	ASSERT(ifcPropertySingleValueInstance);

	return	ifcPropertySingleValueInstance;
}

int_t		buildPropertySingleValue(char * name, char * description, double nominalValue)
{
	return	buildPropertySingleValue(name, description, nominalValue, "IFCREAL");
}

int_t		buildPropertySingleValue(char * name, char * description, double nominalValue, char * typePath)
{
	int_t		ifcPropertySingleValueInstance;
    void    * nominalValueADB;

	ifcPropertySingleValueInstance = sdaiCreateInstanceBN(model, "IFCPROPERTYSINGLEVALUE");

	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Description", sdaiSTRING, description);

	nominalValueADB = sdaiCreateADB(sdaiREAL, (int_t *) &nominalValue);
	sdaiPutADBTypePath(nominalValueADB, 1, typePath); 
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "NominalValue", sdaiADB, (void*) nominalValueADB);

	ASSERT(ifcPropertySingleValueInstance);

	return	ifcPropertySingleValueInstance;
}


int_t		buildPropertySingleValue(char * name, char * description, char * nominalValue)
{
	return	buildPropertySingleValue(name, description, nominalValue, "IFCTEXT");
}

int_t		buildPropertySingleValue(char * name, char * description, char * nominalValue, char * typePath)
{
	int_t		ifcPropertySingleValueInstance;
    void    * nominalValueADB;

	ifcPropertySingleValueInstance = sdaiCreateInstanceBN(model, "IFCPROPERTYSINGLEVALUE");

	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "Description", sdaiSTRING, description);

	nominalValueADB = sdaiCreateADB(sdaiSTRING, nominalValue);
	sdaiPutADBTypePath(nominalValueADB, 1, typePath); 
	sdaiPutAttrBN(ifcPropertySingleValueInstance, "NominalValue", sdaiADB, (void*) nominalValueADB);

	ASSERT(ifcPropertySingleValueInstance);

	return	ifcPropertySingleValueInstance;
}


//
//
//		ElementQuantity, QuantityLength, QuantityArea, QuantityVolume
//
//


int_t		buildElementQuantity(char * name, int_t ** aggrQuantities)
{
	int_t		ifcElementQuantityInstance;

	ifcElementQuantityInstance = sdaiCreateInstanceBN(model, "IFCELEMENTQUANTITY");

	sdaiPutAttrBN(ifcElementQuantityInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcElementQuantityInstance, "OwnerHistory", sdaiINSTANCE, (void*) getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcElementQuantityInstance, "Name", sdaiSTRING, name);

	(* aggrQuantities) = sdaiCreateAggrBN(ifcElementQuantityInstance, "Quantities");

	ASSERT(ifcElementQuantityInstance);

	return	ifcElementQuantityInstance;
}

int_t		buildQuantityLength(char * name, char * description, double length)
{
	int_t		ifcQuantityLengthInstance;

	ifcQuantityLengthInstance = sdaiCreateInstanceBN(model, "IFCQUANTITYLENGTH");

	sdaiPutAttrBN(ifcQuantityLengthInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcQuantityLengthInstance, "Description", sdaiSTRING, description);
	sdaiPutAttrBN(ifcQuantityLengthInstance, "LengthValue", sdaiREAL, &length);

	ASSERT(ifcQuantityLengthInstance);

	return	ifcQuantityLengthInstance;
}

int_t		buildQuantityArea(char * name, char * description, double area)
{
	int_t		ifcQuantityAreaInstance;

	ifcQuantityAreaInstance = sdaiCreateInstanceBN(model, "IFCQUANTITYAREA");

	sdaiPutAttrBN(ifcQuantityAreaInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcQuantityAreaInstance, "Description", sdaiSTRING, description);
	sdaiPutAttrBN(ifcQuantityAreaInstance, "AreaValue", sdaiREAL, &area);

	ASSERT(ifcQuantityAreaInstance);

	return	ifcQuantityAreaInstance;
}

int_t		buildQuantityVolume(char * name, char * description, double volume)
{
	int_t		ifcQuantityVolumeInstance;

	ifcQuantityVolumeInstance = sdaiCreateInstanceBN(model, "IFCQUANTITYVOLUME");

	sdaiPutAttrBN(ifcQuantityVolumeInstance, "Name", sdaiSTRING, name);
	sdaiPutAttrBN(ifcQuantityVolumeInstance, "Description", sdaiSTRING, description);
	sdaiPutAttrBN(ifcQuantityVolumeInstance, "VolumeValue", sdaiREAL, &volume);

	ASSERT(ifcQuantityVolumeInstance);

	return	ifcQuantityVolumeInstance;
}


//
//
//		Pset_WallCommon, BaseQuantities_Wall, BaseQuantities_WallStandardCase, BaseQuantities_Opening, Pset_WindowCommon, BaseQuantities_Window
//
//


int_t		buildPset_WallCommon()
{
    int_t     ifcPropertySetInstance, * aggrHasProperties;
    
    ifcPropertySetInstance = buildPropertySet("Pset_WallCommon", &aggrHasProperties);

	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Reference", "Reference", "", "IFCIDENTIFIER"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("AcousticRating", "AcousticRating", "", "IFCLABEL"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("FireRating", "FireRating", "", "IFCLABEL"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Combustible", "Combustible", false));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SurfaceSpreadOfFlame", "SurfaceSpreadOfFlame", "", "IFCLABEL"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("ThermalTransmittance", "ThermalTransmittance", 0.24, "IFCTHERMALTRANSMITTANCEMEASURE"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("IsExternal", "IsExternal", true));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("ExtendToStructure", "ExtendToStructure", false));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("LoadBearing", "LoadBearing", false));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Compartmentation", "Compartmentation", false));

	ASSERT(ifcPropertySetInstance);

	return	ifcPropertySetInstance;
}

int_t		buildBaseQuantities_Wall(double width, double length, double height, double openingArea, double linearConversionFactor)
{
    int_t     ifcElementQuantityInstance, * aggrQuantities;

    double  grossSideArea = (length / linearConversionFactor) * (height / linearConversionFactor),
            netSideArea = grossSideArea - openingArea;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Lenght", "Lenght", length));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("GrossSideArea", "GrossSideArea", grossSideArea));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("NetSideArea", "NetSideArea", netSideArea));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("GrossVolume", "GrossVolume", grossSideArea * (width / linearConversionFactor)));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("NetVolume", "NetVolume", netSideArea * (width / linearConversionFactor)));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));

	ASSERT(ifcElementQuantityInstance);

	return	ifcElementQuantityInstance;
}

int_t		buildBaseQuantities_WallStandardCase(double width, double length, double height, double openingArea, double linearConversionFactor)
{
    int_t     ifcElementQuantityInstance, * aggrQuantities;

    double  grossSideArea = (length / linearConversionFactor) * (height / linearConversionFactor),
            netSideArea = grossSideArea - openingArea;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Width", "Width", width));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Lenght", "Lenght", length));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("GrossSideArea", "GrossSideArea", grossSideArea));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("NetSideArea", "NetSideArea", netSideArea));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("GrossVolume", "GrossVolume", grossSideArea * (width / linearConversionFactor)));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityVolume("NetVolume", "NetVolume", netSideArea * (width / linearConversionFactor)));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityArea("GrossFootprintArea", "GrossFootprintArea", (length / linearConversionFactor) * (width / linearConversionFactor)));

	ASSERT(ifcElementQuantityInstance);

	return	ifcElementQuantityInstance;
}

int_t		buildBaseQuantities_Opening(double depth, double height, double width)
{
    int_t     ifcElementQuantityInstance, * aggrQuantities;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Depth", "Depth", depth));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Width", "Width", width));

	ASSERT(ifcElementQuantityInstance);

	return	ifcElementQuantityInstance;
}

int_t		buildPset_DoorCommon()
{
    int_t     ifcPropertySetInstance, * aggrHasProperties;
    
    ifcPropertySetInstance = buildPropertySet("Pset_DoorCommon", &aggrHasProperties);
//??????????????????????????????
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Reference", "Reference", ""));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("FireRating", "FireRating", ""));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("AcousticRating", "AcousticRating", ""));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SecurityRating", "SecurityRating", ""));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("IsExternal", "IsExternal", true));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Infiltration", "Infiltration", false));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("ThermalTransmittance", "ThermalTransmittance", 0.24));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("GlazingAresFraction", "GlazingAresFraction", 0.7));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("HandicapAccessible", "HandicapAccessible", false));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("FireExit", "FireExit", false));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SelfClosing", "SelfClosing", false));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SmokeStop", "SmokeStop", false));

	return	ifcPropertySetInstance;
}

int_t		buildPset_WindowCommon()
{
    int_t     ifcPropertySetInstance, * aggrHasProperties;
    
    ifcPropertySetInstance = buildPropertySet("Pset_WindowCommon", &aggrHasProperties);

	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Reference", "Reference", "", "IFCIDENTIFIER"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("FireRating", "FireRating", "", "IFCLABEL"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("AcousticRating", "AcousticRating", "", "IFCLABEL"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SecurityRating", "SecurityRating", "", "IFCLABEL"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("IsExternal", "IsExternal", true));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("Infiltration", "Infiltration", 0.3, "IFCVOLUMETRICFLOWRATEMEASURE"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("ThermalTransmittance", "ThermalTransmittance", 0.24, "IFCTHERMALTRANSMITTANCEMEASURE"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("GlazingAreaFraction", "GlazingAreaFraction", 0.7, "IFCPOSITIVERATIOMEASURE"));
	sdaiAppend(aggrHasProperties, sdaiINSTANCE, (void *) buildPropertySingleValue("SmokeStop", "SmokeStop", false));

	ASSERT(ifcPropertySetInstance);

	return	ifcPropertySetInstance;
}

int_t		buildBaseQuantities_Window(double height, double width)
{
    int_t     ifcElementQuantityInstance, * aggrQuantities;
    
    ifcElementQuantityInstance = buildElementQuantity("BaseQuantities", &aggrQuantities);

	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Height", "Height", height));
	sdaiAppend(aggrQuantities, sdaiINSTANCE, (void *) buildQuantityLength("Width", "Width", width));

	ASSERT(ifcElementQuantityInstance);

	return	ifcElementQuantityInstance;
}


//
//
//      RelDefinesByProperties
//
//


int_t		buildRelDefinesByProperties(int_t relatedObject, int_t relatingPropertyDefinition)
{
	int_t		ifcRelDefinesByPropertiesInstance, * aggrRelatedObjects;

	ifcRelDefinesByPropertiesInstance = sdaiCreateInstanceBN(model, "IFCRELDEFINESBYPROPERTIES");

	sdaiPutAttrBN(ifcRelDefinesByPropertiesInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcRelDefinesByPropertiesInstance, "OwnerHistory", sdaiINSTANCE, (void*) getOwnerHistoryInstance());

	aggrRelatedObjects = sdaiCreateAggrBN(ifcRelDefinesByPropertiesInstance, "RelatedObjects");
	sdaiAppend(aggrRelatedObjects, sdaiINSTANCE, (void *) relatedObject);
	sdaiPutAttrBN(ifcRelDefinesByPropertiesInstance, "RelatingPropertyDefinition", sdaiINSTANCE, (void *) relatingPropertyDefinition);

	ASSERT(ifcRelDefinesByPropertiesInstance);

	return	ifcRelDefinesByPropertiesInstance;
}
