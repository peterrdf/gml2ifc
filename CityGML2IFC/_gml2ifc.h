#pragma once

#include "../include/engine.h"
#include "../include/ifcengine.h"

#ifdef _WINDOWS
#include "gisengine.h"
#include "_string.h"
#endif

#ifdef __EMSCRIPTEN__
#include "../gisengine/gisengine.h"
#endif

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
#include <set>
using namespace std;

// ************************************************************************************************
template<class T>
class _auto_var
{

private: // Fields

	T& m_var;
	const T& m_resetVal;

public: // Methods	

	_auto_var(T& var, const T& initVal, const T& resetVal)
		: m_var(var)		
		, m_resetVal(resetVal)
	{
		m_var = initVal;
	}

	_auto_var(T& var, const T& resetVal)
		: m_var(var)
		, m_resetVal(resetVal)
	{
	}

	virtual ~_auto_var()
	{
		m_var = m_resetVal;
	}
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
struct _vector3d
{
	double x;
	double y;
	double z;
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

	_matrix()
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

	~_matrix()
	{}
};

// ************************************************************************************************
class CSRSTransformer
{

public: // methods

	virtual void toWGS84Async(int iCRS, float fX, float fY, float fZ) = 0;
	virtual const char* getWGS84(int iCRS, float fX, float fY, float fZ) = 0;
};

// ************************************************************************************************
class _gml2ifc_exporter;

// ************************************************************************************************
class _material
{

private: // Members

	int m_iR;
	int m_iG;
	int m_iB;
	int m_iA;

public: // Methods

	_material(int iR, int iG, int iB, int iA)
		: m_iR(iR)
		, m_iG(iG)
		, m_iB(iB)
		, m_iA(iA)
	{}

	_material(const vector<string>& vecRGBA)
		: m_iR(atoi(vecRGBA[0].c_str()))
		, m_iG(atoi(vecRGBA[1].c_str()))
		, m_iB(atoi(vecRGBA[2].c_str()))
		, m_iA(atoi(vecRGBA[3].c_str()))
	{}

	int getR() const { return m_iR; }
	int getG() const { return m_iG; }
	int getB() const { return m_iB; }
	int getA() const { return m_iA; }
};

// ************************************************************************************************
class _settings_provider
{

private: // Members

	_gml2ifc_exporter* m_pSite;

	// Entity or $ALL : _material*
	map<string, _material*> m_mapDefaultMaterials;
	map<string, _material*> m_mapOverridenMaterials;

public: // Methods

	_settings_provider(_gml2ifc_exporter* pSite, const wstring& strSettingsFile);
	virtual ~_settings_provider();

	const map<string, _material*>& getDefaultMaterials() const { return m_mapDefaultMaterials; }
	const map<string, _material*>& getOverridenMaterials() const { return m_mapOverridenMaterials; }

private: // Methods

	void loadSettings(const wstring& strSettingsFile);

	_gml2ifc_exporter* getSite() const { return m_pSite; }
};

// ************************************************************************************************
class _gml2ifc_exporter
{

private: // Members

	wstring m_strRootFolder;
	_settings_provider* m_pSettingsProvider;
	_log_callback m_pLogCallback;
	CSRSTransformer* m_pSRSTransformer;
	OwlModel m_iOwlModel;
	OwlInstance m_iOwlRootInstance;

public: // Methods

	_gml2ifc_exporter(
		const wstring& strRootFolder, 
		_log_callback pLogCallback,
		CSRSTransformer* pSRSTransformer);
	virtual ~_gml2ifc_exporter();

	// pre-processing
	int retrieveSRSData(const wstring& strInputFile);
	int retrieveSRSData(unsigned char* szData, size_t iSize);

	// export
	void execute(const wstring& strInputFile, const wstring& strOuputFile);
	void execute(unsigned char* szData, size_t iSize, const wstring& strOuputFile);

	// SRS
	bool toWGS84Async(int iCRS, float fX, float fY, float fZ);
	bool getWGS84(int iCRS, float fX, float fY, float fZ, string& strCoordinates);

	// Settings
	_material* getDefaultMaterial(const string& strEntity);
	_material* getOverridenMaterial(const string& strEntity);

	// Log
	static string dateTimeStamp();
	static string addDateTimeStamp(const string& strInput);
	void logWrite(enumLogEvent enLogEvent, const string& strEvent);
	void logInfo(const string& strEvent) { logWrite(enumLogEvent::info, strEvent); }
	void logWarn(const string& strEvent) { logWrite(enumLogEvent::warning, strEvent); }
	void logErr(const string& strEvent) { logWrite(enumLogEvent::error, strEvent); }

	OwlModel getOwlModel() const { return m_iOwlModel; }
	OwlInstance getOwlRootInstance() const { return m_iOwlRootInstance; }

private: // Methods

	void setFormatSettings(OwlModel iOwlModel);
	int retrieveSRSDataCore(OwlInstance iRootInstance);
	void executeCore(OwlInstance iRootInstance, const wstring& strOuputFile);	
};

// ************************************************************************************************
class _exporter_base
{

private: // Members

	_gml2ifc_exporter* m_pSite;

	RdfProperty m_iTagProperty;

	SdaiModel m_iSdaiModel;	
	SdaiInstance m_iPersonInstance;
	SdaiInstance m_iOrganizationInstance;
	SdaiInstance m_iPersonAndOrganizationInstance;	
	SdaiInstance m_iApplicationInstance;
	SdaiInstance m_iOwnerHistoryInstance;
	SdaiInstance m_iDimensionalExponentsInstance;
	SdaiInstance m_iConversionBasedUnitInstance;
	SdaiInstance m_iUnitAssignmentInstance;
	SdaiInstance m_iWorldCoordinateSystemInstance;
	SdaiInstance m_iProjectInstance;
	SdaiInstance m_iSiteInstance;
	SdaiInstance m_iSiteInstancePlacement;	
	SdaiInstance m_iGeometricRepresentationContextInstance;

public: // Methods

	_exporter_base(_gml2ifc_exporter* pSite);
	virtual ~_exporter_base();

	// pre-processing
	virtual int retrieveSRSData(OwlInstance iRootInstance) { return 0; }

	// export
	void execute(OwlInstance iRootInstance, const wstring& strOuputFile);

	_gml2ifc_exporter* getSite() const { return m_pSite; }
	SdaiModel getSdaiModel() const { return m_iSdaiModel; }
	SdaiInstance getPersonInstance();
	SdaiInstance getOrganizationInstance();
	SdaiInstance getPersonAndOrganizationInstance();
	SdaiInstance getApplicationInstance();
	SdaiInstance getOwnerHistoryInstance();
	SdaiInstance getDimensionalExponentsInstance();
	SdaiInstance getConversionBasedUnitInstance();
	SdaiInstance getUnitAssignmentInstance();
	SdaiInstance getWorldCoordinateSystemInstance();	
	SdaiInstance getProjectInstance();
	SdaiInstance getSiteInstance(SdaiInstance& iSiteInstancePlacement);
	SdaiInstance getGeometricRepresentationContextInstance();

protected: // Methods

	virtual void preProcessing() {}
	virtual void executeCore(OwlInstance iRootInstance, const wstring& strOuputFile) = 0;
	virtual void postProcessing() {}

	virtual void onPreCreateSite(_matrix* pSiteMatrix) {}
	virtual void onPostCreateSite(SdaiInstance iSiteInstance) {}

	/* Model */
	void createIfcModel(const wchar_t* szSchemaName);
	void saveIfcFile(const wchar_t* szFileName);

	/* Geometry */
	SdaiInstance buildSIUnitInstance(const char* szUnitType, const char* szPrefix, const char* szName);
	SdaiInstance buildMeasureWithUnitInstance();
	SdaiInstance buildDirectionInstance2D(double dX, double dY);
	SdaiInstance buildDirectionInstance3D(double dX, double dY, double dZ);
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
	SdaiInstance buildRelNestsInstance(
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

	/* CRS */
	SdaiInstance buildMapConversion(OwlInstance iSourceCRSInstance, OwlInstance iTargetCRSInstance);
	SdaiInstance buildProjectedCRS(const string& strEPSG);
	string getEPSGCode(const string& strSrsName);
	string getEPSG(const string& strSrsName);

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
	string getStringAttributeValue(OwlInstance iInstance, const string& strName) const;
	string getStringPropertyValue(OwlInstance iInstance, const string& strName) const;
	void getDoublePropertyValue(OwlInstance iInstance, const string& strName, vector<double>& vecValue) const;
	OwlInstance* getObjectProperty(OwlInstance iInstance, const string& strPropertyName, int64_t& iInstancesCount) const;
	bool hasObjectProperty(OwlInstance iInstance, const string& strPropertyName);
	void getPosValues(const string& strContent, vector<double>& vecValues) const;
	void getPosValuesW(const wstring& strContent, vector<double>& vecValues) const;
};

// ************************************************************************************************
class _citygml_exporter : public _exporter_base
{

private: // Members

	// World
	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;

	// Geometry Kernel
	OwlClass m_iPoint3DClass;
	OwlClass m_iCollectionClass;
	OwlClass m_iTransformationClass;	
	map<OwlInstance, bool> m_mapInstanceDefaultState;
	map<OwlInstance, vector<SdaiInstance>> m_mapMappedItems; // OwlInstance : Geometries

	// CRS
	OwlClass m_iCityModelClass;
	OwlClass m_iBoundingShapeClass;
	OwlClass m_iEnvelopeClass;	
	OwlInstance m_iEnvelopeInstance;
	map<OwlInstance, OwlInstance> m_mapBuildingSRS; // Building : Envelope
	map<OwlInstance, OwlInstance> m_mapParcelSRS; // Parcel : Reference Point		

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
	map<OwlInstance, vector<OwlInstance>> m_mapBuildingElements; // Building Supported Element : Geometries
	
	// Parcel
	OwlClass m_iCadastralParcelClass;
	OwlClass m_iPointPropertyClass;
	OwlClass m_iReferencePointIndicatorClass;

	// Feature
	OwlClass m_iVegetationObjectClass;
	OwlClass m_iWaterObjectClass;
	OwlClass m_iBridgeObjectClass;
	OwlClass m_iTunnelObjectClass;
	OwlClass m_iTransportationObjectClass;
	OwlClass m_iFurnitureObjectClass;
	OwlClass m_iReliefObjectClass;
	OwlClass m_iLandUseClass;
	OwlClass m_iTrafficSpaceClass;
	OwlClass m_iTrafficAreaClass;
	OwlClass m_iThingClass; // Unknown
	map<OwlInstance, vector<OwlInstance>> m_mapFeatures; // Feature : Supported Elements
	map<OwlInstance, vector<OwlInstance>> m_mapFeatureElements; // Feature Supported Element : Geometries

	// Sites
	vector<SdaiInstance> m_vecSiteInstances;
	
	 // Temp
	OwlInstance m_iCurrentOwlBuildingElementInstance;
	double m_dXOffset;
	double m_dYOffset;
	double m_dZOffset;

	// Cache	
	SdaiInstance m_iDefaultWallSurfaceColorRgbInstance;
	SdaiInstance m_iDefaultRoofSurfaceColorRgbInstance;
	SdaiInstance m_iDefaultDoorColorRgbInstance;
	SdaiInstance m_iDefaultWindowColorRgbInstance;
	SdaiInstance m_iDefaultColorRgbInstance;

public: // Methods

	_citygml_exporter(_gml2ifc_exporter* pSite);
	virtual ~_citygml_exporter();

	virtual int retrieveSRSData(OwlInstance iRootInstance) override;

protected:  // Methods	

	virtual void preProcessing() override;
	virtual void executeCore(OwlInstance iRootInstance, const wstring& strOuputFile) override;
	virtual void postProcessing() override;

	virtual void onPreCreateSite(_matrix* pSiteMatrix) override;
	virtual void onPostCreateSite(SdaiInstance iSiteInstance) override;

	virtual void createDefaultStyledItemInstance(SdaiInstance iSdaiInstance) override;

	// SRS
	virtual void collectSRSData(OwlInstance iRootInstance);
	virtual void createSRSMapConversion();
	virtual void getXYZOffset(double& dX, double& dY, double& dZ);

	// Buildings
	void createBuildings(SdaiInstance iSiteInstance, SdaiInstance iSiteInstancePlacement);
	void createBuildings();
	void createBuildingsRecursively(OwlInstance iInstance);
	void searchForBuildingElements(OwlInstance iBuildingInstance, OwlInstance iInstance);
	void searchForProxyBuildingElements(OwlInstance iBuildingInstance, OwlInstance iInstance);
	void searchForBuildingElementGeometry(OwlInstance iBuildingElementInstance, OwlInstance iInstance);

	// Features
	void createFeatures(SdaiInstance iSiteInstance, SdaiInstance iSiteInstancePlacement);
	void createFeatures();
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
	void createReferencePointIndicator(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation);
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
	bool IsPoint3DClass(OwlClass iInstanceClass) const;
	bool isCollectionClass(OwlClass iInstanceClass) const;	
	bool isTransformationClass(OwlClass iInstanceClass) const;	
	void getInstancesDefaultState();
	void getInstanceDefaultStateRecursive(OwlInstance iInstance);

	// CRS
	OwlClass isCityModelClass(OwlClass iInstanceClass) const;
	OwlClass isBoundingShapeClass(OwlClass iInstanceClass) const;
	OwlClass isEnvelopeClass(OwlClass iInstanceClass) const;
		
	// Building
	bool isBuildingElement(OwlInstance iInstance) const;
	bool isBuildingClass(OwlClass iInstanceClass) const;
	bool isWallSurfaceClass(OwlClass iInstanceClass) const;
	bool isRoofSurfaceClass(OwlInstance iInstanceClass) const;
	bool isDoorClass(OwlInstance iInstanceClass) const;
	bool isWindowClass(OwlInstance iInstanceClass) const;

	// Parcel
	bool isCadastralParcelClass(OwlClass iInstanceClass) const;
	bool isPointPropertyClass(OwlClass iInstanceClass) const;
	bool isReferencePointIndicatorClass(OwlClass iInstanceClass) const;

	// Feature
	bool isFeatureClass(OwlInstance iInstanceClass) const;
	bool isVegetationObjectClass(OwlClass iInstanceClass) const;
	bool isWaterObjectClass(OwlClass iInstanceClass) const;
	bool isBridgeObjectClass(OwlClass iInstanceClass) const;
	bool isTunnelObjectClass(OwlClass iInstanceClass) const;
	bool isTransportationObjectClass(OwlClass iInstanceClass) const;
	bool isFurnitureObjectClass(OwlClass iInstanceClass) const;
	bool isReliefObjectClass(OwlClass iInstanceClass) const;
	bool isLandUseClass(OwlClass iInstanceClass) const;
	bool isTrafficSpaceClass(OwlClass iInstanceClass) const;
	bool isTrafficAreaClass(OwlClass iInstanceClass) const;
	bool isUnknownClass(OwlClass iInstanceClass) const;

private: // Methods

	void getEnvelopeCenter(OwlInstance iEnvelopeInstance, double& dX, double& dY, double& dZ);
	void getReferencePoint(OwlInstance iReferencePointInstance, double& dX, double& dY, double& dZ);
	void setSiteEnvelopeSRSData(SdaiInstance iSiteInstance, OwlInstance iEnvelopeInstance);
	void setSiteReferencePointSRSData(SdaiInstance iSiteInstance, OwlInstance iReferencePointInstance);	
	bool retrieveEnvelopeSRSData(OwlInstance iEnvelopeInstance, string& strEPSGCode, vector<double>& vecLowerCorner, vector<double>& vecUpperCorner);
	bool retrieveEnvelopeSRSData(OwlInstance iEnvelopeInstance, string& strEPSGCode, vector<double>& vecCenter);
	bool transformEnvelopeSRSDataAsync(OwlInstance iEnvelopeInstance);
	bool retrieveReferencePointSRSData(OwlInstance iReferencePointInstance, string& strEPSGCode, vector<double>& vecCenter);
	bool transformReferencePointSRSDataAsync(OwlInstance iReferencePointInstance);
};

// ************************************************************************************************
class _gml_exporter : public _citygml_exporter
{

public: // Methods

	_gml_exporter(_gml2ifc_exporter* pSite);
	virtual ~_gml_exporter();
};

// ************************************************************************************************
class _cityjson_exporter : public _citygml_exporter
{

private: // Members

	OwlClass m_iCityJSONClass;
	OwlClass m_iMetadataClass;
	OwlInstance m_iMetadataInstance;

public: // Methods

	_cityjson_exporter(_gml2ifc_exporter* pSite);
	virtual ~_cityjson_exporter();

	virtual int retrieveSRSData(OwlInstance iRootInstance) override;

protected: // Methods

	virtual void onPreCreateSite(_matrix* pSiteMatrix) override;
	virtual void onPostCreateSite(SdaiInstance iSiteInstance) override;

	virtual void collectSRSData(OwlInstance iRootInstance) override;
	virtual void createSRSMapConversion() override;
	virtual void getXYZOffset(double& dX, double& dY, double& dZ) override;

	// CRS
	OwlClass isCityJSONClass(OwlClass iInstanceClass) const;
	OwlClass isMetadataClass(OwlClass iInstanceClass) const;

private: // Methods

	void getMetadataCenter(OwlInstance iMetadataInstance, double& dX, double& dY, double& dZ);
	bool transformMetadataSRSDataAsync(OwlInstance iMetadataInstance);
	bool retrieveMetadataSRSData(OwlInstance iMetadataInstance, string& strEPSGCode, vector<double>& vecLowerCorner, vector<double>& vecUpperCorner);
	bool retrieveMetadataSRSData(OwlInstance iMetadataInstance, string& strEPSGCode, vector<double>& vecCenter);
	void setSiteMetadataSRSData(SdaiInstance iSiteInstance, OwlInstance iMetadataInstance);
};