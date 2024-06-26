#pragma once

#include "engine.h"
#include "gisengine.h"
#include "ifcengine.h"

#include "_guid.h"

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <time.h>
#include <map>
using namespace std;

// ************************************************************************************************
class _gis2ifc
{

private: // Members

	wstring m_strRootFolder;
	_log_callback m_pLogCallback;
	OwlModel m_iOwlModel;

public: // Methods

	_gis2ifc(const wstring& strRootFolder, _log_callback pLogCallback);
	virtual ~_gis2ifc();

	void execute(const wstring& strInputFile, const wstring& strOuputFile);

	// Log
	static string dateTimeStamp();
	static string addDateTimeStamp(const string& strInput);
	void logWrite(enumLogEvent enLogEvent, const string& strEvent);
	void logInfo(const string& strEvent) { logWrite(enumLogEvent::info, strEvent); }
	void logWarn(const string& strEvent) { logWrite(enumLogEvent::warning, strEvent); }
	void logErr(const string& strEvent) { logWrite(enumLogEvent::error, strEvent); }

	OwlModel getOwlModel() const { return m_iOwlModel; }

private: // Methods

	void setFormatSettings(OwlModel iOwlModel);
};

// ************************************************************************************************
class _point3d 
{

public: // Members

	double X;
	double Y;
	double Z;

public: // Methods

	_point3d()
		: X(0.)
		, Y(0.)
		, Z(0.)
	{}
};

// ************************************************************************************************
class _matrix
{

public: // Members

	double _11;
	double _12;
	double _13;
	double _21;
	double _22;
	double _23;
	double _31;
	double _32;
	double _33;
	double _41;
	double _42;
	double _43;

public: // Methods

	_matrix::_matrix()
		: _11(1.)
		, _12(0.)
		, _13(0.)
		, _21(0.)
		, _22(1.)
		, _23(0.)
		, _31(0.)
		, _32(0.)
		, _33(1.)
		, _41(0.)
		, _42(0.)
		, _43(0.)
	{}

	_matrix::~_matrix()
	{}
};

// ************************************************************************************************
class _exporter_base
{

private: // Members

	_gis2ifc* m_pSite;

	RdfProperty m_iTagProperty;

	SdaiModel m_iIfcModel;	
	SdaiInstance m_iSiteInstance;
	SdaiInstance m_iPersonInstance;
	SdaiInstance m_iOrganizationInstance;
	SdaiInstance m_iPersonAndOrganizationInstance;	
	SdaiInstance m_iApplicationInstance;
	SdaiInstance m_iOwnerHistoryInstance;
	SdaiInstance m_iDimensionalExponentsInstance;
	SdaiInstance m_iConversionBasedUnitInstance;
	SdaiInstance m_iUnitAssignmentInstance;
	SdaiInstance m_iWorldCoordinateSystemInstance;
	SdaiInstance m_iGeometricRepresentationContextInstance;
	SdaiInstance m_iProjectInstance;	

public: // Methods

	_exporter_base(_gis2ifc* pSite);
	virtual ~_exporter_base();

	virtual void execute(OwlInstance iRootInstance, const wstring& strOuputFile) = 0;

	_gis2ifc* getSite() const { return m_pSite; }
	SdaiModel getIfcModel() const { return m_iIfcModel; }
	SdaiInstance getPersonInstance();
	SdaiInstance getOrganizationInstance();
	SdaiInstance getPersonAndOrganizationInstance();
	SdaiInstance getApplicationInstance();
	SdaiInstance getOwnerHistoryInstance();
	SdaiInstance getDimensionalExponentsInstance();
	SdaiInstance getConversionBasedUnitInstance();
	SdaiInstance getUnitAssignmentInstance();
	SdaiInstance getWorldCoordinateSystemInstance();
	SdaiInstance getGeometricRepresentationContextInstance();
	SdaiInstance getProjectInstance();

protected: // Methods	

	/* Model */
	void createIfcModel(const wchar_t* szSchemaName);
	void saveIfcFile(const wchar_t* szFileName);

	/* Geometry */
	SdaiInstance buildSIUnitInstance(const char* szUnitType, const char* szPrefix, const char* szName);
	SdaiInstance buildMeasureWithUnitInstance();
	SdaiInstance buildDirectionInstance(double dX, double dY, double dZ);
	SdaiInstance buildCartesianPointInstance(double dX, double dY, double dZ);
	SdaiInstance buildSiteInstance(
		const char* szName,
		const char* szDescription,
		_matrix* pMatrix, 
		SdaiInstance& iSiteInstancePlacement);
	SdaiInstance buildLocalPlacementInstance(_matrix* pMatrix, SdaiInstance iPlacementRelativeTo);
	SdaiInstance buildAxis2Placement3DInstance(_matrix* pMatrix);	
	SdaiInstance buildBuildingInstance(
		const char* szName,
		const char* szDescription,
		_matrix* pMatrix, 
		SdaiInstance iPlacementRelativeTo, 
		SdaiInstance& iBuildingInstancePlacement);
	SdaiInstance buildFeatureInstance(
		const char* szName,
		const char* szDescription,
		_matrix* pMatrix,
		SdaiInstance iPlacementRelativeTo,
		SdaiInstance& iBuildingInstancePlacement,
		const vector<SdaiInstance>& vecRepresentations);
	SdaiInstance buildTransportElementInstance(
		const char* szName,
		const char* szDescription,
		_matrix* pMatrix,
		SdaiInstance iPlacementRelativeTo,
		SdaiInstance& iBuildingInstancePlacement,
		const vector<SdaiInstance>& vecRepresentations);
	SdaiInstance buildFurnitureObjectInstance(
		const char* szName,
		const char* szDescription,
		_matrix* pMatrix,
		SdaiInstance iPlacementRelativeTo,
		SdaiInstance& iBuildingInstancePlacement,
		const vector<SdaiInstance>& vecRepresentations);
	SdaiInstance buildGeographicElementInstance(
		const char* szName,
		const char* szDescription,
		_matrix* pMatrix,
		SdaiInstance iPlacementRelativeTo,
		SdaiInstance& iBuildingInstancePlacement,
		const vector<SdaiInstance>& vecRepresentations);
	SdaiInstance buildBuildingStoreyInstance(_matrix* pMatrix, SdaiInstance iPlacementRelativeTo, SdaiInstance& iBuildingStoreyInstancePlacement);	
	SdaiInstance buildProductDefinitionShapeInstance(const vector<SdaiInstance>& vecRepresentations);
	SdaiInstance buildRelAggregatesInstance(
		const char* szName, 
		const char* szDescription, 
		SdaiInstance iRelatingObjectInstance, 
		const vector<SdaiInstance>& vecRelatedObjects);
	SdaiInstance buildRelContainedInSpatialStructureInstance(
		const char* szName,
		const char* szDescription,
		SdaiInstance iRelatingStructureInstance,
		const vector<SdaiInstance>& vecRelatedElements);
	SdaiInstance buildBuildingElementInstance(
		const char* szEntity,
		const char* szName,
		const char* szDescription,
		_matrix* pMatrix,
		SdaiInstance iPlacementRelativeTo,
		SdaiInstance& iBuildingElementInstancePlacement,
		const vector<SdaiInstance>& vecRepresentations);
	SdaiInstance buildRepresentationMap(_matrix* pMatrix, const vector<SdaiInstance>& vecRepresentations);
	SdaiInstance buildMappedItem(
		const vector<SdaiInstance>& vecRepresentations,
		OwlInstance iReferencePointMatrixInstance,
		OwlInstance iTransformationMatrixInstance);

	/* Style */
	void createStyledItemInstance(OwlInstance iOwlInstance, SdaiInstance iSdaiInstance);
	void createStyledItemInstance(SdaiInstance iSdaiInstance, double dR, double G, double dB, double dTransparency);
	void createStyledItemInstance(SdaiInstance iSdaiInstance, SdaiInstance iColorRgbInstance, double dTransparency);
	virtual void createDefaultStyledItemInstance(SdaiInstance iSdaiInstance) {}
	SdaiInstance buildPresentationStyleAssignmentInstance();
	SdaiInstance buildSurfaceStyleInstance();
	SdaiInstance buildSurfaceStyleRenderingInstance();
	SdaiInstance buildColorRgbInstance(double dR, double dG, double dB);

	/* Properties */
	SdaiInstance buildPropertySet(char* szName, SdaiAggr& pHasProperties);
	SdaiInstance buildRelDefinesByProperties(SdaiInstance iRelatedObject, SdaiInstance iRelatingPropertyDefinition);
	SdaiInstance buildPropertySingleValueText(
		const char* szName, 
		const char* szDescription,
		const char* szNominalValue,
		const char* szTypePath);
	SdaiInstance buildPropertySingleValueReal(
		const char* szName,
		const char* szDescription,
		double dNominalValue,
		const char* szTypePath);

	/* Materials */
	SdaiInstance buildMaterial();
	SdaiInstance buildMaterialLayer(double dThickness);
	SdaiInstance buildMaterialLayerSet(double dThickness);
	SdaiInstance buildMaterialLayerSetUsage(double dThickness);
	SdaiInstance buildRelAssociatesMaterial(SdaiInstance iBuildingElementInstance, double dThickness);

	/* Helpers */
	string getTag(OwlInstance iInstance) const;
	OwlInstance* getObjectProperty(OwlInstance iInstance, const string& strPropertyName, int64_t& iInstancesCount) const;
	bool hasObjectProperty(OwlInstance iInstance, const string& strPropertyName);
};

// ************************************************************************************************
class _citygml_exporter : public _exporter_base
{

private: // Members

	// Geometry Kernel
	OwlClass m_iCollectionClass;
	OwlClass m_iTransformationClass;
	map<OwlInstance, vector<SdaiInstance>> m_mapMappedItems; // OwlInstance : Geometries

	// CityObjectGroup
	OwlClass m_iCityObjectGroupMemberClass;

	// relativeGMLGeometry (class:GeometryPropertyType => class:geometryMember)
	OwlClass m_iGeometryMemberClass;

	// Building
	OwlClass m_iBuildingClass;
	OwlClass m_iWallSurfaceClass;
	OwlClass m_iRoofSurfaceClass;
	OwlClass m_iDoorClass;
	OwlClass m_iWindowClass;
	map<OwlInstance, vector<OwlInstance>> m_mapBuildings; // Building : Supported Elements
	map<OwlInstance, vector<OwlInstance>> m_mapBuildingElements; // Building (Supported) Element : Geometries

	// Feature
	OwlClass m_iVegetationObjectClass;
	OwlClass m_iWaterObjectClass;
	OwlClass m_iBridgeObjectClass;
	OwlClass m_iTunnelObjectClass;
	OwlClass m_iTransportationObjectClass;
	OwlClass m_iFurnitureObjectClass;
	OwlClass m_iReliefObjectClass;
	OwlClass m_iThingClass; // Unknown
	map<OwlInstance, vector<OwlInstance>> m_mapFeatures; // Feature : Supported Elements
	map<OwlInstance, vector<OwlInstance>> m_mapFeatureElements; // Feature (Supported) Element : Geometries
	
	 // Temp
	OwlInstance m_iCurrentOwlBuildingElementInstance;

	// Cache
	SdaiInstance m_iDefaultWallSurfaceColorRgbInstance;
	SdaiInstance m_iDefaultRoofSurfaceColorRgbInstance;
	SdaiInstance m_iDefaultDoorColorRgbInstance;
	SdaiInstance m_iDefaultWindowColorRgbInstance;
	SdaiInstance m_iDefaultColorRgbInstance;

public: // Methods

	_citygml_exporter(_gis2ifc* pSite);
	virtual ~_citygml_exporter();

	virtual void execute(OwlInstance iRootInstance, const wstring& strOuputFile) override;

protected:  // Methods

	virtual void createDefaultStyledItemInstance(SdaiInstance iSdaiInstance) override;

	// Buildings
	void createBuildings(SdaiInstance iSiteInstance, SdaiInstance iSiteInstancePlacement);
	void createBuildingsRecursively(OwlInstance iInstance);
	void searchForBuildingElements(OwlInstance iBuildingInstance, OwlInstance iInstance);
	void searchForProxyBuildingElements(OwlInstance iBuildingInstance, OwlInstance iInstance);
	void searchForBuildingElementGeometry(OwlInstance iBuildingElementInstance, OwlInstance iInstance);

	// Features
	void createFeatures(SdaiInstance iSiteInstance, SdaiInstance iSiteInstancePlacement);
	void createFeaturesRecursively(OwlInstance iInstance);
	void searchForFeatureElements(OwlInstance iFeatureInstance, OwlInstance iInstance);

	// Geometry
	void createGeometry(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createSolid(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createCompositeSolid(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createMultiSolid(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createMultiSurface(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createCompositeSurface(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createSurfaceMember(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createBoundaryRepresentation(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createPoint3D(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createPoint3DSet(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
	void createPolyLine3D(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);

	void createProperties(OwlInstance iOwlInstance, SdaiInstance iSdaiInstance);

	SdaiInstance buildBuildingElementInstance(
		OwlInstance iOwlInstance,
		_matrix* pMatrix,
		SdaiInstance iPlacementRelativeTo,
		SdaiInstance& iBuildingElementInstancePlacement,
		const vector<SdaiInstance>& vecRepresentations);

	// Geometry Kernel
	bool isCollectionClass(OwlClass iInstanceClass) const;
	bool isTransformationClass(OwlClass iInstanceClass) const;
	
	// Building
	bool isBuildingElement(OwlInstance iInstance) const;
	bool isBuildingClass(OwlClass iInstanceClass) const;
	bool isWallSurfaceClass(OwlClass iInstanceClass) const;
	bool isRoofSurfaceClass(OwlInstance iInstanceClass) const;
	bool isDoorClass(OwlInstance iInstanceClass) const;
	bool isWindowClass(OwlInstance iInstanceClass) const;

	// Feature
	bool isFeatureClass(OwlInstance iInstanceClass) const;
	bool isVegetationObjectClass(OwlClass iInstanceClass) const;
	bool isWaterObjectClass(OwlClass iInstanceClass) const;
	bool isBridgeObjectClass(OwlClass iInstanceClass) const;
	bool isTunnelObjectClass(OwlClass iInstanceClass) const;
	bool isTransportationObjectClass(OwlClass iInstanceClass) const;
	bool isFurnitureObjectClass(OwlClass iInstanceClass) const;
	bool isReliefObjectClass(OwlClass iInstanceClass) const;
	bool isUnknownClass(OwlClass iInstanceClass) const;
};

// ************************************************************************************************
class _gml_exporter : public _citygml_exporter
{
public: // Methods

	_gml_exporter(_gis2ifc* pSite);
	virtual ~_gml_exporter();
};

// ************************************************************************************************
class _cityjson_exporter : public _citygml_exporter
{
public: // Methods

	_cityjson_exporter(_gis2ifc* pSite);
	virtual ~_cityjson_exporter();
};