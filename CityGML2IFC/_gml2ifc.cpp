#include "pch.h"
#include "_gml2ifc.h"

#ifdef _WINDOWS
#include "_string.h"
#endif

#include <float.h>
#include <locale>
#include <codecvt>
#include <cassert>

// ************************************************************************************************
_gml2ifc_exporter::_gml2ifc_exporter(
		const wstring& strRootFolder,
		_log_callback pLogCallback,
		CSRSTransformer* pSRSTransformer)
	: m_strRootFolder(strRootFolder)
	, m_pLogCallback(pLogCallback)
	, m_pSRSTransformer(pSRSTransformer)
	, m_iOwlModel(0)
{
	assert(!m_strRootFolder.empty());
	assert(m_pLogCallback != nullptr);

	SetGISOptionsW(strRootFolder.c_str(), true, m_pLogCallback);
}

/*virtual*/ _gml2ifc_exporter::~_gml2ifc_exporter()
{
	if (m_iOwlModel != 0)
	{
		CloseModel(m_iOwlModel);
		m_iOwlModel = 0;
	}
}

int _gml2ifc_exporter::retrieveSRSData(const wstring& strInputFile)
{
	assert(!strInputFile.empty());

	/* Import */
	if (m_iOwlModel != 0)
	{
		CloseModel(m_iOwlModel);
		m_iOwlModel = 0;
	}

	m_iOwlModel = CreateModel();
	assert(m_iOwlModel != 0);

	setFormatSettings(m_iOwlModel);

	OwlInstance iRootInstance = ImportGISModelW(m_iOwlModel, strInputFile.c_str());
	if (iRootInstance != 0)
	{
		return retrieveSRSDataCore(iRootInstance);
	}
	else
	{
		logErr("Not supported format.");
	}

	return 0;
}

int _gml2ifc_exporter::retrieveSRSData(unsigned char* szData, size_t iSize)
{
	assert(szData != nullptr);
	assert(iSize > 0);

	/* Import */
	if (m_iOwlModel != 0)
	{
		CloseModel(m_iOwlModel);
		m_iOwlModel = 0;
	}

	m_iOwlModel = CreateModel();
	assert(m_iOwlModel != 0);

	setFormatSettings(m_iOwlModel);

	OwlInstance iRootInstance = ImportGISModelA(m_iOwlModel, szData, iSize);
	if (iRootInstance != 0)
	{
		return retrieveSRSDataCore(iRootInstance);
	}
	else
	{
		logErr("Not supported format.");
	}

	return 0;
}

void _gml2ifc_exporter::execute(const wstring& strInputFile, const wstring& strOuputFile)
{
	assert(!strInputFile.empty());
	assert(!strOuputFile.empty());

	/* Import */
	if (m_iOwlModel != 0)
	{
		CloseModel(m_iOwlModel);
		m_iOwlModel = 0;
	}

	m_iOwlModel = CreateModel();
	assert(m_iOwlModel != 0);

	setFormatSettings(m_iOwlModel);

	OwlInstance iRootInstance = ImportGISModelW(m_iOwlModel, strInputFile.c_str());
	if (iRootInstance != 0)
	{
		executeCore(iRootInstance, strOuputFile);
	}
	else
	{
		logErr("Not supported format.");
	}
}

void _gml2ifc_exporter::execute(unsigned char* szData, size_t iSize, const wstring& strOuputFile)
{
	assert(szData != nullptr);
	assert(iSize > 0);
	assert(!strOuputFile.empty());

	/* Import */
	if (m_iOwlModel != 0)
	{
		CloseModel(m_iOwlModel);
		m_iOwlModel = 0;
	}

	m_iOwlModel = CreateModel();
	assert(m_iOwlModel != 0);

	setFormatSettings(m_iOwlModel);

	OwlInstance iRootInstance = ImportGISModelA(m_iOwlModel, szData, iSize);
	if (iRootInstance != 0)
	{
		executeCore(iRootInstance, strOuputFile);
	}
	else
	{
		logErr("Not supported format.");
	}
}

/*static*/ string _gml2ifc_exporter::dateTimeStamp()
{
	auto timePointNow = chrono::system_clock::now();
	auto timeNow = chrono::system_clock::to_time_t(timePointNow);
	auto timeNowMS = chrono::duration_cast<chrono::milliseconds>(timePointNow.time_since_epoch()) % 1000;

	stringstream ss;
	ss << put_time(localtime(&timeNow), "%Y-%m-%d %H:%M:%S.");
	ss << setfill('0') << setw(3) << timeNowMS.count();

	return ss.str();
}

bool _gml2ifc_exporter::toWGS84Async(int iCRS, float fX, float fY, float fZ)
{
	if (m_pSRSTransformer != nullptr)
	{
		m_pSRSTransformer->toWGS84Async(iCRS, fX, fY, fZ);

		return true;
	}

	return false;
}

bool _gml2ifc_exporter::getWGS84(int iCRS, float fX, float fY, float fZ, string& strCoordinates)
{
	strCoordinates = "";

	if (m_pSRSTransformer != nullptr)
	{
		const char* szCoordinates = m_pSRSTransformer->getWGS84(iCRS, fX, fY, fZ);
		strCoordinates = szCoordinates != nullptr ? szCoordinates : "";

		return !strCoordinates.empty();
	}

	return false;
}

/*static*/ string _gml2ifc_exporter::addDateTimeStamp(const string& strInput)
{
	string strInputCopy = dateTimeStamp();
	strInputCopy += ": ";
	strInputCopy += strInput;

	return strInputCopy;
}

void _gml2ifc_exporter::logWrite(enumLogEvent enLogEvent, const string& strEvent)
{
	(*m_pLogCallback)(enLogEvent, addDateTimeStamp(strEvent).c_str());
}

void _gml2ifc_exporter::setFormatSettings(OwlModel iOwlModel)
{
	string strSettings = "111111000000001111000001110001";

	bitset<64> bitSettings(strSettings);
	int64_t iSettings = bitSettings.to_ulong();

	string strMask = "11111111111111111111011101110111";
	bitset<64> bitMask(strMask);
	int64_t iMask = bitMask.to_ulong();

	SetFormat(iOwlModel, (int64_t)iSettings, (int64_t)iMask);

	SetBehavior(iOwlModel, 2048 + 4096, 2048 + 4096);
}

void _gml2ifc_exporter::executeCore(OwlInstance iRootInstance, const wstring& strOuputFile)
{
	assert(iRootInstance != 0);
	assert(!strOuputFile.empty());

	logInfo("Exporting...");

	if (IsGML(m_iOwlModel))
	{
		_gml_exporter exporter(this);
		exporter.execute(iRootInstance, strOuputFile);

		logInfo("Done.");
	}
	else if (IsCityGML(m_iOwlModel))
	{
		_citygml_exporter exporter(this);
		exporter.execute(iRootInstance, strOuputFile);

		logInfo("Done.");
	}
	else if (IsCityJSON(m_iOwlModel))
	{
		_cityjson_exporter exporter(this);
		exporter.execute(iRootInstance, strOuputFile);

		logInfo("Done.");
	}
	else
	{
		logErr("Not supported format.");
	}
}

int _gml2ifc_exporter::retrieveSRSDataCore(OwlInstance iRootInstance)
{
	assert(iRootInstance != 0);

	logInfo("Retrieving SRS Data...");

	int iTransformationsCount = 0;

	if (IsGML(m_iOwlModel))
	{
		_gml_exporter exporter(this);
		iTransformationsCount = exporter.retrieveSRSData(iRootInstance);

		logInfo("Done.");
	}
	else if (IsCityGML(m_iOwlModel))
	{
		_citygml_exporter exporter(this);
		iTransformationsCount = exporter.retrieveSRSData(iRootInstance);

		logInfo("Done.");
	}
	else if (IsCityJSON(m_iOwlModel))
	{
		_cityjson_exporter exporter(this);
		iTransformationsCount = exporter.retrieveSRSData(iRootInstance);

		logInfo("Done.");
	}
	else
	{
		logErr("Not supported format.");
	}

	return iTransformationsCount;
}

// ************************************************************************************************
_exporter_base::_exporter_base(_gml2ifc_exporter* pSite)
	: m_pSite(pSite)
	, m_iTagProperty(0)
	, m_iSdaiModel(0)	
	, m_iSiteInstance(0)
	, m_iPersonInstance(0)
	, m_iOrganizationInstance(0)
	, m_iPersonAndOrganizationInstance(0)	
	, m_iApplicationInstance(0)
	, m_iOwnerHistoryInstance(0)
	, m_iDimensionalExponentsInstance(0)
	, m_iConversionBasedUnitInstance(0)
	, m_iUnitAssignmentInstance(0)
	, m_iWorldCoordinateSystemInstance(0)
	, m_iProjectInstance(0)
	
{
	assert(m_pSite != nullptr);

	m_iTagProperty = GetPropertyByName(getSite()->getOwlModel(), "tag");
	assert(m_iTagProperty);
}

/*virtual*/ _exporter_base::~_exporter_base()
{
	if (m_iSdaiModel != 0)
	{
		sdaiCloseModel(m_iSdaiModel);
		m_iSdaiModel = 0;
	}
}

void _exporter_base::execute(OwlInstance iRootInstance, const wstring& strOuputFile)
{
	preProcessing();

	executeCore(iRootInstance, strOuputFile);

	postProcessing();
}

SdaiInstance _exporter_base::getPersonInstance()
{
	if (m_iPersonInstance == 0) 
	{
		m_iPersonInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcPerson");
		assert(m_iPersonInstance != 0);

		sdaiPutAttrBN(m_iPersonInstance, "GivenName", sdaiSTRING, "Peter");
		sdaiPutAttrBN(m_iPersonInstance, "FamilyName", sdaiSTRING, "Bonsma");
	}

	return	m_iPersonInstance;
}

SdaiInstance _exporter_base::getOrganizationInstance()
{
	if (m_iOrganizationInstance == 0) 
	{
		m_iOrganizationInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcOrganization");
		assert(m_iOrganizationInstance != 0);

		sdaiPutAttrBN(m_iOrganizationInstance, "Name", sdaiSTRING, "RDF");
		sdaiPutAttrBN(m_iOrganizationInstance, "Description", sdaiSTRING, "RDF Ltd.");
	}	

	return	m_iOrganizationInstance;
}

SdaiInstance _exporter_base::getPersonAndOrganizationInstance()
{
	if (m_iPersonAndOrganizationInstance == 0) 
	{
		m_iPersonAndOrganizationInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcPersonAndOrganization");
		assert(m_iPersonAndOrganizationInstance != 0);

		sdaiPutAttrBN(m_iPersonAndOrganizationInstance, "ThePerson", sdaiINSTANCE, (void*)getPersonInstance());
		sdaiPutAttrBN(m_iPersonAndOrganizationInstance, "TheOrganization", sdaiINSTANCE, (void*)getOrganizationInstance());
	}

	return	m_iPersonAndOrganizationInstance;
}

SdaiInstance _exporter_base::getApplicationInstance()
{
	if (m_iApplicationInstance == 0)
	{
		m_iApplicationInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcApplication");
		assert(m_iApplicationInstance != 0);

		sdaiPutAttrBN(m_iApplicationInstance, "ApplicationDeveloper", sdaiINSTANCE, (void*)getOrganizationInstance());
		sdaiPutAttrBN(m_iApplicationInstance, "Version", sdaiSTRING, "0.10"); //#tbd
		sdaiPutAttrBN(m_iApplicationInstance, "ApplicationFullName", sdaiSTRING, "Test Application"); //#tbd
		sdaiPutAttrBN(m_iApplicationInstance, "ApplicationIdentifier", sdaiSTRING, "TA 1001"); //#tbd
	}

	return	m_iApplicationInstance;
}

SdaiInstance _exporter_base::getOwnerHistoryInstance()
{
	if (m_iOwnerHistoryInstance == 0)
	{
		int64_t iTimeStamp = time(0);

		m_iOwnerHistoryInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcOwnerHistory");
		assert(m_iOwnerHistoryInstance != 0);

		sdaiPutAttrBN(m_iOwnerHistoryInstance, "OwningUser", sdaiINSTANCE, (void*)getPersonAndOrganizationInstance());
		sdaiPutAttrBN(m_iOwnerHistoryInstance, "OwningApplication", sdaiINSTANCE, (void*)getApplicationInstance());
		sdaiPutAttrBN(m_iOwnerHistoryInstance, "ChangeAction", sdaiENUM, "ADDED");
		sdaiPutAttrBN(m_iOwnerHistoryInstance, "CreationDate", sdaiINTEGER, &iTimeStamp);
		sdaiPutAttrBN(m_iOwnerHistoryInstance, "LastModifiedDate", sdaiINTEGER, &iTimeStamp);
	}

	return	m_iOwnerHistoryInstance;
}

SdaiInstance _exporter_base::getDimensionalExponentsInstance()
{
	if (m_iDimensionalExponentsInstance == 0)
	{
		int_t LengthExponent = 0,
			MassExponent = 0,
			TimeExponent = 0,
			ElectricCurrentExponent = 0,
			ThermodynamicTemperatureExponent = 0,
			AmountOfSubstanceExponent = 0,
			LuminousIntensityExponent = 0;

		m_iDimensionalExponentsInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcDimensionalExponents");
		assert(m_iDimensionalExponentsInstance != 0);

		sdaiPutAttrBN(m_iDimensionalExponentsInstance, "LengthExponent", sdaiINTEGER, &LengthExponent);
		sdaiPutAttrBN(m_iDimensionalExponentsInstance, "MassExponent", sdaiINTEGER, &MassExponent);
		sdaiPutAttrBN(m_iDimensionalExponentsInstance, "TimeExponent", sdaiINTEGER, &TimeExponent);
		sdaiPutAttrBN(m_iDimensionalExponentsInstance, "ElectricCurrentExponent", sdaiINTEGER, &ElectricCurrentExponent);
		sdaiPutAttrBN(m_iDimensionalExponentsInstance, "ThermodynamicTemperatureExponent", sdaiINTEGER, &ThermodynamicTemperatureExponent);
		sdaiPutAttrBN(m_iDimensionalExponentsInstance, "AmountOfSubstanceExponent", sdaiINTEGER, &AmountOfSubstanceExponent);
		sdaiPutAttrBN(m_iDimensionalExponentsInstance, "LuminousIntensityExponent", sdaiINTEGER, &LuminousIntensityExponent);
	}

	return	m_iDimensionalExponentsInstance;
}

SdaiInstance _exporter_base::getConversionBasedUnitInstance()
{
	if (m_iConversionBasedUnitInstance == 0)
	{
		m_iConversionBasedUnitInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcConversionBasedUnit");
		assert(m_iConversionBasedUnitInstance != 0);

		sdaiPutAttrBN(m_iConversionBasedUnitInstance, "Dimensions", sdaiINSTANCE, (void*)getDimensionalExponentsInstance());
		sdaiPutAttrBN(m_iConversionBasedUnitInstance, "UnitType", sdaiENUM, "PLANEANGLEUNIT");
		sdaiPutAttrBN(m_iConversionBasedUnitInstance, "Name", sdaiSTRING, "DEGREE");
		sdaiPutAttrBN(m_iConversionBasedUnitInstance, "ConversionFactor", sdaiINSTANCE, (void*)buildMeasureWithUnitInstance());
	}

	return	m_iConversionBasedUnitInstance;
}

SdaiInstance _exporter_base::getUnitAssignmentInstance()
{
	if (m_iUnitAssignmentInstance == 0)
	{
		m_iUnitAssignmentInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcUnitAssignment");
		assert(m_iUnitAssignmentInstance != 0);

		SdaiAggr pUnits = sdaiCreateAggrBN(m_iUnitAssignmentInstance, "Units");
		assert(pUnits != nullptr);

		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("LENGTHUNIT", nullptr, "METRE"));
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("AREAUNIT", nullptr, "SQUARE_METRE"));
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("VOLUMEUNIT", nullptr, "CUBIC_METRE"));
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)getConversionBasedUnitInstance());
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("SOLIDANGLEUNIT", nullptr, "STERADIAN"));
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("MASSUNIT", nullptr, "GRAM"));
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("TIMEUNIT", nullptr, "SECOND"));
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("THERMODYNAMICTEMPERATUREUNIT", nullptr, "DEGREE_CELSIUS"));
		sdaiAppend(pUnits, sdaiINSTANCE, (void*)buildSIUnitInstance("LUMINOUSINTENSITYUNIT", nullptr, "LUMEN"));
	}

	return	m_iUnitAssignmentInstance;
}

SdaiInstance _exporter_base::getWorldCoordinateSystemInstance()
{
	if (m_iWorldCoordinateSystemInstance == 0)
	{
		m_iWorldCoordinateSystemInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcAxis2Placement3D");
		assert(m_iWorldCoordinateSystemInstance != 0);

		sdaiPutAttrBN(m_iWorldCoordinateSystemInstance, "Location", sdaiINSTANCE, (void*)buildCartesianPointInstance(0., 0., 0.));
	}	

	return m_iWorldCoordinateSystemInstance;
}

SdaiInstance _exporter_base::getProjectInstance()
{
	if (m_iProjectInstance == 0) 
	{
		m_iProjectInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcProject");
		assert(m_iProjectInstance != 0);

		sdaiPutAttrBN(m_iProjectInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
		sdaiPutAttrBN(m_iProjectInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
		sdaiPutAttrBN(m_iProjectInstance, "Name", sdaiSTRING, "Default Project"); //#tbd
		sdaiPutAttrBN(m_iProjectInstance, "Description", sdaiSTRING, "Description of Default Project"); //#tbd
		sdaiPutAttrBN(m_iProjectInstance, "UnitsInContext", sdaiINSTANCE, (void*)getUnitAssignmentInstance());

		SdaiAggr pRepresentationContexts = sdaiCreateAggrBN(m_iProjectInstance, "RepresentationContexts");
		assert(pRepresentationContexts != nullptr);

		SdaiInstance iGeometricRepresentationContextInstance = buildGeometricRepresentationContextInstance();
		assert(iGeometricRepresentationContextInstance != 0);

		// CRS #todo
		//OwlInstance iModelEnvelopeInstance = getModelEnvelopeInstance();
		//if (iModelEnvelopeInstance != 0)
		//{
		//	const wchar_t* szSrsName = getStringAttributeValue(iModelEnvelopeInstance, "srsName");
		//	if ((szSrsName != nullptr) && (wstring(szSrsName).find(L"EPSG") != string::npos))
		//	{
		//		string strEPSG = getEPSG(szSrsName);
		//		assert(!strEPSG.empty());

		//		SdaiInstance iSourceCRS = buildProjectedCRS(strEPSG);
		//		SdaiInstance iTargetCRS = buildProjectedCRS(strEPSG);
		//		SdaiInstance iMapConversion = buildMapConversion(iSourceCRS, iTargetCRS);

		//		double dOrthogonalHeight = 10000; // #todo
		//		sdaiPutAttrBN(iMapConversion, "OrthogonalHeight", sdaiREAL, &dOrthogonalHeight);

		//		double dEastings = 0.; // #todo
		//		sdaiPutAttrBN(iMapConversion, "Eastings", sdaiREAL, &dEastings);

		//		double dNorthings = 1.; // #todo
		//		sdaiPutAttrBN(iMapConversion, "Northings", sdaiREAL, &dNorthings);

		//		sdaiPutAttrBN(iGeometricRepresentationContextInstance, "HasCoordinateOperation", sdaiINSTANCE, (void*)iMapConversion);
		//	} // if ((szSrsName != nullptr) && ...
		//} // if (iModelEnvelopeInstance != 0)

		sdaiAppend(pRepresentationContexts, sdaiINSTANCE, (void*)iGeometricRepresentationContextInstance);
	}

	return m_iProjectInstance;
}

void _exporter_base::createIfcModel(const wchar_t* szSchemaName)
{
	assert(szSchemaName != nullptr);

	if (m_iSdaiModel != 0)
	{
		sdaiCloseModel(m_iSdaiModel);
		m_iSdaiModel = 0;
	}	

	m_iSdaiModel = sdaiCreateModelBNUnicode(1, NULL, szSchemaName);
	assert(m_iSdaiModel != 0);

	//#tbd
	char    description[512], timeStamp[512];
	time_t  t;
	struct tm* tInfo;

	time(&t);
	tInfo = localtime(&t);

	//#tbd
	if (true)//view == COORDINATIONVIEW) {
		//if (m_Quantities.GetCheck()) {
		memcpy(description, "ViewDefinition [CoordinationView, QuantityTakeOffAddOnView]", sizeof("ViewDefinition [CoordinationView, QuantityTakeOffAddOnView]"));
	//}
	/*else {
		memcpy(description, "ViewDefinition [CoordinationView]", sizeof("ViewDefinition [CoordinationView]"));
	}*/
	/*}
	else {
		ASSERT(view == PRESENTATIONVIEW);
		if (m_Quantities.GetCheck()) {
			memcpy(description, "ViewDefinition [PresentationView, QuantityTakeOffAddOnView]", sizeof("ViewDefinition [PresentationView, QuantityTakeOffAddOnView]"));
		}
		else {
			memcpy(description, "ViewDefinition [PresentationView]", sizeof("ViewDefinition [PresentationView]"));
		}
	}*/

	sprintf(&timeStamp[0], "%d", 1900 + tInfo->tm_year);
	sprintf(&timeStamp[4], "%d", 100 + 1 + tInfo->tm_mon);
	sprintf(&timeStamp[7], "%d", 100 + tInfo->tm_mday);
	timeStamp[4] = '-';
	timeStamp[7] = '-';
	sprintf(&timeStamp[10], "%d", 100 + tInfo->tm_hour);
	sprintf(&timeStamp[13], "%d", 100 + tInfo->tm_min);
	sprintf(&timeStamp[16], "%d", 100 + tInfo->tm_sec);
	timeStamp[10] = 'T';
	timeStamp[13] = ':';
	timeStamp[16] = ':';
	timeStamp[19] = 0;

	SetSPFFHeader(
		m_iSdaiModel,
		(const char*)description,           //  description //#tbd
		"2;1",                              //  implementationLevel //#tbd
		(const char*)nullptr,	            //  name //#tbd
		(const char*)&timeStamp[0],         //  timeStamp //#tbd
		"Architect",                        //  author //#tbd
		"Building Designer Office",         //  organization //#tbd
		"IFC Engine DLL version 1.03 beta", //  preprocessorVersion //#tbd
		"IFC Engine DLL version 1.03 beta", //  originatingSystem //#tbd
		"The authorising person",           //  authorization //#tbd
#ifdef _WINDOWS
		CW2A(szSchemaName)                  //  fileSchema //#tbd
#else
#ifdef __EMSCRIPTEN__
		(LPCSTR)CW2A(szSchemaName)                  //  fileSchema //#tbd
#else
#error NOT IMPLEMENTED!
#endif
#endif		
	);
}

void _exporter_base::saveIfcFile(const wchar_t* szFileName)
{
	assert(szFileName != nullptr);
	assert(m_iSdaiModel != 0);

	sdaiSaveModelBNUnicode(m_iSdaiModel, szFileName);
}

SdaiInstance _exporter_base::buildGeometricRepresentationContextInstance()
{
	double dPrecision = 0.00001;
	int_t iCoordinateSpaceDimension = 3;

	SdaiInstance iGeometricRepresentationContextInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcGeometricRepresentationContext");
	assert(iGeometricRepresentationContextInstance != 0);

	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "ContextType", sdaiSTRING, "Model");
	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "CoordinateSpaceDimension", sdaiINTEGER, &iCoordinateSpaceDimension);
	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "Precision", sdaiREAL, &dPrecision);
	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "WorldCoordinateSystem", sdaiINSTANCE, (void*)getWorldCoordinateSystemInstance());
	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "TrueNorth", sdaiINSTANCE, (void*)buildDirectionInstance2D(0., 1.));

	return iGeometricRepresentationContextInstance;
}

SdaiInstance _exporter_base::buildSIUnitInstance(const char* szUnitType, const char* szPrefix, const char* szName)
{
	assert(szUnitType != nullptr);
	assert(szName != nullptr);

	SdaiInstance iSIUnitInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcSIUnit");
	assert(iSIUnitInstance != 0);

	sdaiPutAttrBN(iSIUnitInstance, "Dimensions", sdaiINTEGER, (void*)nullptr);
	sdaiPutAttrBN(iSIUnitInstance, "UnitType", sdaiENUM, szUnitType);
	if (szPrefix != nullptr) 
	{
		sdaiPutAttrBN(iSIUnitInstance, "Prefix", sdaiENUM, szPrefix);
	}
	sdaiPutAttrBN(iSIUnitInstance, "Name", sdaiENUM, szName);

	return iSIUnitInstance;
}

SdaiInstance _exporter_base::buildMeasureWithUnitInstance()
{
	double	dValueComponent = 0.01745; //#tbd
	SdaiADB pValueComponentADB = sdaiCreateADB(sdaiREAL, &dValueComponent);
	assert(pValueComponentADB != nullptr);

	SdaiInstance iMeasureWithUnitInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcMeasureWithUnit");
	assert(iMeasureWithUnitInstance != 0);	

	sdaiPutADBTypePath(pValueComponentADB, 1, "IFCREAL");
	sdaiPutAttrBN(iMeasureWithUnitInstance, "ValueComponent", sdaiADB, (void*)pValueComponentADB);
	sdaiPutAttrBN(iMeasureWithUnitInstance, "UnitComponent", sdaiINSTANCE, (void*)buildSIUnitInstance("PLANEANGLEUNIT", NULL, "RADIAN"));
	
	return iMeasureWithUnitInstance;
}

SdaiInstance _exporter_base::buildDirectionInstance2D(double dX, double dY)
{
	SdaiInstance iDirectionInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcDirection");
	assert(iDirectionInstance != 0);

	SdaiAggr pDirectionRatios = sdaiCreateAggrBN(iDirectionInstance, "DirectionRatios");
	assert(pDirectionRatios != nullptr);

	sdaiAppend(pDirectionRatios, sdaiREAL, &dX);
	sdaiAppend(pDirectionRatios, sdaiREAL, &dY);

	return iDirectionInstance;
}

SdaiInstance _exporter_base::buildDirectionInstance3D(double dX, double dY, double dZ)
{
	SdaiInstance iDirectionInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcDirection");
	assert(iDirectionInstance != 0);

	SdaiAggr pDirectionRatios = sdaiCreateAggrBN(iDirectionInstance, "DirectionRatios");
	assert(pDirectionRatios != nullptr);

	sdaiAppend(pDirectionRatios, sdaiREAL, &dX);
	sdaiAppend(pDirectionRatios, sdaiREAL, &dY);
	sdaiAppend(pDirectionRatios, sdaiREAL, &dZ);

	return iDirectionInstance;
}

SdaiInstance _exporter_base::buildCartesianPointInstance(double dX, double dY, double dZ)
{
	SdaiInstance iCartesianPointInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcCartesianPoint");
	assert(iCartesianPointInstance != 0);

	SdaiAggr pCoordinates = sdaiCreateAggrBN(iCartesianPointInstance, "Coordinates");
	assert(pCoordinates != nullptr);

	sdaiAppend(pCoordinates, sdaiREAL, &dX);
	sdaiAppend(pCoordinates, sdaiREAL, &dY);
	sdaiAppend(pCoordinates, sdaiREAL, &dZ);

	return iCartesianPointInstance;
}

SdaiInstance _exporter_base::buildSiteInstance(
	const char* szName,
	const char* szDescription,
	_matrix* pMatrix, 
	SdaiInstance& iSiteInstancePlacement)
{
	assert(pMatrix != nullptr);

	SdaiInstance iSiteInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcSite");
	assert(iSiteInstance != 0);

	sdaiPutAttrBN(iSiteInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iSiteInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iSiteInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iSiteInstance, "Description", sdaiSTRING, szDescription);

	iSiteInstancePlacement = buildLocalPlacementInstance(pMatrix, 0);
	assert(iSiteInstancePlacement != 0);

	sdaiPutAttrBN(iSiteInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iSiteInstancePlacement);
	sdaiPutAttrBN(iSiteInstance, "CompositionType", sdaiENUM, "ELEMENT");

	return iSiteInstance;
}

SdaiInstance _exporter_base::buildLocalPlacementInstance(_matrix* pMatrix, SdaiInstance iPlacementRelativeTo)
{
	SdaiInstance iLocalPlacementInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcLocalPlacement");
	assert(iLocalPlacementInstance != 0);

	if (iPlacementRelativeTo != 0) 
	{
		sdaiPutAttrBN(iLocalPlacementInstance, "PlacementRelTo", sdaiINSTANCE, (void*)iPlacementRelativeTo);
	}
	sdaiPutAttrBN(iLocalPlacementInstance, "RelativePlacement", sdaiINSTANCE, (void*)buildAxis2Placement3DInstance(pMatrix));

	return iLocalPlacementInstance;
}

SdaiInstance _exporter_base::buildAxis2Placement3DInstance(_matrix* pMatrix)
{
	SdaiInstance iAxis2Placement3DInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcAxis2Placement3D");
	assert(iAxis2Placement3DInstance != 0);

	sdaiPutAttrBN(iAxis2Placement3DInstance, "Location", sdaiINSTANCE, (void*)buildCartesianPointInstance(pMatrix->_41, pMatrix->_42, pMatrix->_43));
	sdaiPutAttrBN(iAxis2Placement3DInstance, "Axis", sdaiINSTANCE, (void*)buildDirectionInstance3D(pMatrix->_31, pMatrix->_32, pMatrix->_33));
	sdaiPutAttrBN(iAxis2Placement3DInstance, "RefDirection", sdaiINSTANCE, (void*)buildDirectionInstance3D(pMatrix->_11, pMatrix->_12, pMatrix->_13));

	return iAxis2Placement3DInstance;
}

SdaiInstance _exporter_base::buildBuildingInstance(
	const char* szName,
	const char* szDescription,
	_matrix* pMatrix,
	SdaiInstance iPlacementRelativeTo,
	SdaiInstance& iBuildingInstancePlacement)
{
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);

	SdaiInstance iBuildingInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcBuilding");
	assert(iBuildingInstance != 0);

	sdaiPutAttrBN(iBuildingInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iBuildingInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iBuildingInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iBuildingInstance, "Description", sdaiSTRING, szDescription);

	iBuildingInstancePlacement = buildLocalPlacementInstance(pMatrix, iPlacementRelativeTo);
	assert(iBuildingInstancePlacement != 0);

	sdaiPutAttrBN(iBuildingInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iBuildingInstancePlacement);
	sdaiPutAttrBN(iBuildingInstance, "CompositionType", sdaiENUM, "ELEMENT");
	//sdaiPutAttrBN(iBuildingInstance, "BuildingAddress", sdaiINSTANCE, (void*)buildPostalAddress()); //#tbd

	return iBuildingInstance;
}

SdaiInstance _exporter_base::buildFeatureInstance(
	const char* szName,
	const char* szDescription,
	_matrix* pMatrix,
	SdaiInstance iPlacementRelativeTo,
	SdaiInstance& iBuildingInstancePlacement,
	const vector<SdaiInstance>& vecRepresentations)
{
	assert(szName != nullptr);
	assert(szDescription != nullptr);
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);
	assert(!vecRepresentations.empty());

	SdaiInstance iBuildingElementInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcBuildingElementProxy");
	assert(iBuildingElementInstance != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iBuildingElementInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iBuildingElementInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iBuildingElementInstance, "Description", sdaiSTRING, szDescription);

	iBuildingInstancePlacement = buildLocalPlacementInstance(pMatrix, iPlacementRelativeTo);
	assert(iBuildingInstancePlacement != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iBuildingInstancePlacement);
	sdaiPutAttrBN(iBuildingElementInstance, "Representation", sdaiINSTANCE, (void*)buildProductDefinitionShapeInstance(vecRepresentations));

	return iBuildingElementInstance;
}

SdaiInstance _exporter_base::buildTransportElementInstance(
	const char* szName,
	const char* szDescription,
	_matrix* pMatrix,
	SdaiInstance iPlacementRelativeTo,
	SdaiInstance& iBuildingInstancePlacement,
	const vector<SdaiInstance>& vecRepresentations)
{
	assert(szName != nullptr);
	assert(szDescription != nullptr);
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);
	assert(!vecRepresentations.empty());

	SdaiInstance iBuildingElementInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcTransportElement");
	assert(iBuildingElementInstance != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iBuildingElementInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iBuildingElementInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iBuildingElementInstance, "Description", sdaiSTRING, szDescription);

	iBuildingInstancePlacement = buildLocalPlacementInstance(pMatrix, iPlacementRelativeTo);
	assert(iBuildingInstancePlacement != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iBuildingInstancePlacement);
	sdaiPutAttrBN(iBuildingElementInstance, "Representation", sdaiINSTANCE, (void*)buildProductDefinitionShapeInstance(vecRepresentations));

	return iBuildingElementInstance;
}

SdaiInstance _exporter_base::buildFurnitureObjectInstance(
	const char* szName,
	const char* szDescription,
	_matrix* pMatrix,
	SdaiInstance iPlacementRelativeTo,
	SdaiInstance& iBuildingInstancePlacement,
	const vector<SdaiInstance>& vecRepresentations)
{
	assert(szName != nullptr);
	assert(szDescription != nullptr);
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);
	assert(!vecRepresentations.empty());

	SdaiInstance iBuildingElementInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcFurnishingElement");
	assert(iBuildingElementInstance != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iBuildingElementInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iBuildingElementInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iBuildingElementInstance, "Description", sdaiSTRING, szDescription);

	iBuildingInstancePlacement = buildLocalPlacementInstance(pMatrix, iPlacementRelativeTo);
	assert(iBuildingInstancePlacement != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iBuildingInstancePlacement);
	sdaiPutAttrBN(iBuildingElementInstance, "Representation", sdaiINSTANCE, (void*)buildProductDefinitionShapeInstance(vecRepresentations));

	return iBuildingElementInstance;
}

SdaiInstance _exporter_base::buildGeographicElementInstance(
	const char* szName,
	const char* szDescription,
	_matrix* pMatrix,
	SdaiInstance iPlacementRelativeTo,
	SdaiInstance& iBuildingInstancePlacement,
	const vector<SdaiInstance>& vecRepresentations)
{
	assert(szName != nullptr);
	assert(szDescription != nullptr);
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);
	assert(!vecRepresentations.empty());

	SdaiInstance iBuildingElementInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcGeographicElement");
	assert(iBuildingElementInstance != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iBuildingElementInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iBuildingElementInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iBuildingElementInstance, "Description", sdaiSTRING, szDescription);

	iBuildingInstancePlacement = buildLocalPlacementInstance(pMatrix, iPlacementRelativeTo);
	assert(iBuildingInstancePlacement != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iBuildingInstancePlacement);
	sdaiPutAttrBN(iBuildingElementInstance, "Representation", sdaiINSTANCE, (void*)buildProductDefinitionShapeInstance(vecRepresentations));

	return iBuildingElementInstance;
}

SdaiInstance _exporter_base::buildBuildingStoreyInstance(_matrix* pMatrix, SdaiInstance iPlacementRelativeTo, SdaiInstance& iBuildingStoreyInstancePlacement)
{
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);

	SdaiInstance iBuildingStoreyInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcBuildingStorey");
	assert(iBuildingStoreyInstance != 0);

	sdaiPutAttrBN(iBuildingStoreyInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iBuildingStoreyInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iBuildingStoreyInstance, "Name", sdaiSTRING, "Default Building Storey"); //#tbd
	sdaiPutAttrBN(iBuildingStoreyInstance, "Description", sdaiSTRING, "Description of Default Building Storey"); //#tbd

	iBuildingStoreyInstancePlacement = buildLocalPlacementInstance(pMatrix, iPlacementRelativeTo);
	assert(iBuildingStoreyInstancePlacement != 0);

	sdaiPutAttrBN(iBuildingStoreyInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iBuildingStoreyInstancePlacement);
	sdaiPutAttrBN(iBuildingStoreyInstance, "CompositionType", sdaiENUM, "ELEMENT");

	double dElevation = 0;
	sdaiPutAttrBN(iBuildingStoreyInstance, "Elevation", sdaiREAL, &dElevation);

	return iBuildingStoreyInstance;
}

SdaiInstance _exporter_base::buildProductDefinitionShapeInstance(const vector<SdaiInstance>& vecRepresentations)
{
	assert(!vecRepresentations.empty());

	SdaiInstance iProductDefinitionShapeInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcProductDefinitionShape");
	assert(iProductDefinitionShapeInstance != 0);

	SdaiAggr pRepresentations = sdaiCreateAggrBN(iProductDefinitionShapeInstance, "Representations");
	assert(pRepresentations != nullptr);

	for (auto iRepresentation : vecRepresentations)
	{
		sdaiAppend(pRepresentations, sdaiINSTANCE, (void*)iRepresentation);
	}

	return iProductDefinitionShapeInstance;
}

SdaiInstance _exporter_base::buildRelAggregatesInstance(
	const char* szName, 
	const char* szDescription, 
	SdaiInstance iRelatingObjectInstance, 
	const vector<SdaiInstance>& vecRelatedObjects)
{
	assert(iRelatingObjectInstance != 0);
	assert(!vecRelatedObjects.empty());

	SdaiInstance iRelAggregatesInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcRelAggregates");
	assert(iRelAggregatesInstance != 0);

	sdaiPutAttrBN(iRelAggregatesInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iRelAggregatesInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iRelAggregatesInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iRelAggregatesInstance, "Description", sdaiSTRING, szDescription);
	sdaiPutAttrBN(iRelAggregatesInstance, "RelatingObject", sdaiINSTANCE, (void*)iRelatingObjectInstance);

	SdaiAggr pRelatedObjects = sdaiCreateAggrBN(iRelAggregatesInstance, "RelatedObjects");
	assert(pRelatedObjects != nullptr);

	for (auto iRelatedObject : vecRelatedObjects)
	{
		sdaiAppend(pRelatedObjects, sdaiINSTANCE, (void*)iRelatedObject);
	}

	return iRelAggregatesInstance;
}

SdaiInstance _exporter_base::buildRelNestsInstance(
	const char* szName,
	const char* szDescription,
	SdaiInstance iRelatingObjectInstance,
	const vector<SdaiInstance>& vecRelatedObjects)
{
	assert(iRelatingObjectInstance != 0);
	assert(!vecRelatedObjects.empty());

	SdaiInstance iIfcRelNestsInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcRelNests");
	assert(iIfcRelNestsInstance != 0);

	sdaiPutAttrBN(iIfcRelNestsInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iIfcRelNestsInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iIfcRelNestsInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iIfcRelNestsInstance, "Description", sdaiSTRING, szDescription);
	sdaiPutAttrBN(iIfcRelNestsInstance, "RelatingObject", sdaiINSTANCE, (void*)iRelatingObjectInstance);

	SdaiAggr pRelatedObjects = sdaiCreateAggrBN(iIfcRelNestsInstance, "RelatedObjects");
	assert(pRelatedObjects != nullptr);

	for (auto iRelatedObject : vecRelatedObjects)
	{
		sdaiAppend(pRelatedObjects, sdaiINSTANCE, (void*)iRelatedObject);
	}

	return iIfcRelNestsInstance;
}

SdaiInstance _exporter_base::buildRelContainedInSpatialStructureInstance(
	const char* szName,
	const char* szDescription,
	SdaiInstance iRelatingStructureInstance,
	const vector<SdaiInstance>& vecRelatedElements)
{
	assert(iRelatingStructureInstance != 0);
	assert(!vecRelatedElements.empty());

	SdaiInstance iRelContainedInSpatialStructureInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcRelContainedInSpatialStructure");
	assert(iRelContainedInSpatialStructureInstance != 0);

	sdaiPutAttrBN(iRelContainedInSpatialStructureInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iRelContainedInSpatialStructureInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iRelContainedInSpatialStructureInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iRelContainedInSpatialStructureInstance, "Description", sdaiSTRING, szDescription);
	sdaiPutAttrBN(iRelContainedInSpatialStructureInstance, "RelatingStructure", sdaiINSTANCE, (void*)iRelatingStructureInstance);

	SdaiAggr pRelatedElements = sdaiCreateAggrBN(iRelContainedInSpatialStructureInstance, "RelatedElements");
	assert(pRelatedElements != nullptr);

	for (auto iRelatedElement : vecRelatedElements)
	{
		sdaiAppend(pRelatedElements, sdaiINSTANCE, (void*)iRelatedElement);
	}

	return iRelContainedInSpatialStructureInstance;
}

SdaiInstance _exporter_base::buildBuildingElementInstance(
	const char* szEntity,
	const char* szName,
	const char* szDescription,
	_matrix* pMatrix,
	SdaiInstance iPlacementRelativeTo,
	SdaiInstance& iBuildingElementInstancePlacement,
	const vector<SdaiInstance>& vecRepresentations)
{
	assert(szEntity != nullptr);
	assert(szName != nullptr);
	assert(szDescription != nullptr);
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);
	assert(!vecRepresentations.empty());

	SdaiInstance iBuildingElementInstance = sdaiCreateInstanceBN(m_iSdaiModel, szEntity);
	assert(iBuildingElementInstance != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iBuildingElementInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iBuildingElementInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iBuildingElementInstance, "Description", sdaiSTRING, szDescription);

	iBuildingElementInstancePlacement = buildLocalPlacementInstance(pMatrix, iPlacementRelativeTo);
	assert(iBuildingElementInstancePlacement != 0);

	sdaiPutAttrBN(iBuildingElementInstance, "ObjectPlacement", sdaiINSTANCE, (void*)iBuildingElementInstancePlacement);
	sdaiPutAttrBN(iBuildingElementInstance, "Representation", sdaiINSTANCE, (void*)buildProductDefinitionShapeInstance(vecRepresentations));

	return iBuildingElementInstance;
}

SdaiInstance _exporter_base::buildRepresentationMap(_matrix* pMatrix, const vector<SdaiInstance>& vecMappedRepresentations)
{
	SdaiInstance iRepresentationMapInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcRepresentationMap");
	assert(iRepresentationMapInstance != 0);

	sdaiPutAttrBN(iRepresentationMapInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iRepresentationMapInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iRepresentationMapInstance, "MappingOrigin", sdaiINSTANCE, (void*)buildAxis2Placement3DInstance(pMatrix));

	SdaiInstance iShapeRepresentationInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcShapeRepresentation");
	assert(iShapeRepresentationInstance != 0);

	sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
	sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Brep");
	sdaiPutAttrBN(iShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*)buildGeometricRepresentationContextInstance());

	SdaiAggr pItems = sdaiCreateAggrBN(iShapeRepresentationInstance, "Items");
	assert(pItems != 0);

	for (auto iMappedRepresentation : vecMappedRepresentations)
	{
		sdaiAppend(pItems, sdaiINSTANCE, (void*)iMappedRepresentation);
	}

	sdaiPutAttrBN(iRepresentationMapInstance, "MappedRepresentation", sdaiINSTANCE, (void*)iShapeRepresentationInstance);	

	return iRepresentationMapInstance;
}

SdaiInstance _exporter_base::buildMappedItem(
	const vector<SdaiInstance>& vecRepresentations,
	OwlInstance iReferencePointMatrixInstance,
	OwlInstance iTransformationMatrixInstance)
{
	assert(!vecRepresentations.empty());
	assert(iReferencePointMatrixInstance != 0);
	assert(iTransformationMatrixInstance != 0);

	SdaiInstance iMappedItemInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcMappedItem");
	assert(iMappedItemInstance != 0);

	sdaiPutAttrBN(iMappedItemInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iMappedItemInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	// Reference Point (Anchor)
	double dReferencePointX = 0.;
	double dReferencePointY = 0.;
	double dReferencePointZ = 0.;
	{
		int64_t iValuesCount = 0;
		double* pdValues = nullptr;
		GetDatatypeProperty(
			iReferencePointMatrixInstance,
			GetPropertyByName(getSite()->getOwlModel(), "coordinates"),
			(void**)&pdValues,
			&iValuesCount);
		assert(iValuesCount == 12);

		dReferencePointX = pdValues[9];
		dReferencePointY = pdValues[10];
		dReferencePointZ = pdValues[11];
	}

	_matrix mtxReferencePoint;
	mtxReferencePoint._41 = dReferencePointX;
	mtxReferencePoint._42 = dReferencePointY;
	mtxReferencePoint._43 = dReferencePointZ;
	sdaiPutAttrBN(iMappedItemInstance, "MappingSource", sdaiINSTANCE, (void*)buildRepresentationMap(&mtxReferencePoint, vecRepresentations));

	SdaiInstance iCartesianTransformationOperator3DInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcCartesianTransformationOperator3D");
	assert(iCartesianTransformationOperator3DInstance != 0);	

	// Transformation Matrix
	{
		int64_t iValuesCount = 0;
		double* pdValues = nullptr;
		GetDatatypeProperty(
			iTransformationMatrixInstance,
			GetPropertyByName(getSite()->getOwlModel(), "coordinates"),
			(void**)&pdValues,
			&iValuesCount);
		assert(iValuesCount == 12);

		sdaiPutAttrBN(iCartesianTransformationOperator3DInstance, "Axis1", sdaiINSTANCE, (void*)buildDirectionInstance3D(pdValues[0], pdValues[1], pdValues[2]));
		sdaiPutAttrBN(iCartesianTransformationOperator3DInstance, "Axis2", sdaiINSTANCE, (void*)buildDirectionInstance3D(pdValues[3], pdValues[4], pdValues[5]));
		sdaiPutAttrBN(iCartesianTransformationOperator3DInstance, "Axis3", sdaiINSTANCE, (void*)buildDirectionInstance3D(pdValues[6], pdValues[7], pdValues[8]));

		SdaiInstance iLocalOriginInstance = buildCartesianPointInstance(pdValues[9], pdValues[10], pdValues[11]);
		assert(iLocalOriginInstance != 0);

		sdaiPutAttrBN(iCartesianTransformationOperator3DInstance, "LocalOrigin", sdaiINSTANCE, (void*)iLocalOriginInstance);
	}	

	sdaiPutAttrBN(iMappedItemInstance, "MappingTarget", sdaiINSTANCE, (void*)iCartesianTransformationOperator3DInstance);

	SdaiInstance iShapeRepresentationInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcShapeRepresentation");
	assert(iShapeRepresentationInstance != 0);

	sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
	sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "MappedRepresentation");
	sdaiPutAttrBN(iShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*)buildGeometricRepresentationContextInstance());

	SdaiAggr pItems = sdaiCreateAggrBN(iShapeRepresentationInstance, "Items");
	assert(pItems != 0);

	sdaiAppend(pItems, sdaiINSTANCE, (void*)iMappedItemInstance);

	return iShapeRepresentationInstance;
}

SdaiInstance _exporter_base::buildMapConversion(OwlInstance iSourceCRSInstance, OwlInstance iTargetCRSInstance)
{
	assert(iSourceCRSInstance != 0);
	assert(iTargetCRSInstance != 0);

	SdaiInstance iMapConversionInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcMapConversion");
	assert(iMapConversionInstance != 0);

	sdaiPutAttrBN(iMapConversionInstance, "SourceCRS", sdaiINSTANCE, (void*)iSourceCRSInstance);
	sdaiPutAttrBN(iMapConversionInstance, "TargetCRS", sdaiINSTANCE, (void*)iTargetCRSInstance);

	return iMapConversionInstance;
}

SdaiInstance _exporter_base::buildProjectedCRS(const string& strEPSG)
{
	assert(!strEPSG.empty());

	SdaiInstance iProjectedCRSInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcProjectedCRS");
	assert(iProjectedCRSInstance != 0);

	sdaiPutAttrBN(iProjectedCRSInstance, "Name", sdaiSTRING, strEPSG.c_str());

	return iProjectedCRSInstance;
}

string _exporter_base::getEPSGCode(const string& strSrsName)
{
	assert(!strSrsName.empty());

	size_t iIndex = string::npos;

	// srsName="urn:ogc:def:crs,crs:EPSG::31256"
	if ((iIndex = strSrsName.find("EPSG::")) != string::npos)
	{
		assert(false); //#todo
	}

	// EPSG:3763
	if (strSrsName.find("EPSG:") == 0)
	{
		string strCode = strSrsName.substr(iIndex + 6).c_str();

		return strCode;
	}

	assert(false);

	return "";
}

string _exporter_base::getEPSG(const string& strSrsName)
{
	assert(!strSrsName.empty());

	size_t iIndex = string::npos;

	// srsName="urn:ogc:def:crs,crs:EPSG::31256"
	if ((iIndex = strSrsName.find("EPSG::")) != string::npos)
	{
		string strEPSG = strSrsName.substr(iIndex).c_str();
		iIndex = strEPSG.find("::");
		strEPSG.replace(iIndex, 2, ":");

		return strEPSG;
	}

	// EPSG:3763
	if (strSrsName.find("EPSG:") == 0)
	{
		return strSrsName;
	}

	// http://www.opengis.net/def/crs/EPSG/0/25830
	if (strSrsName.find("/EPSG/") != -1)
	{
		iIndex = strSrsName.rfind("/");
		string strEPSG = "EPSG:";
		strEPSG += strSrsName.substr(iIndex + 1).c_str();

		return strEPSG;
	}

	assert(false);

	return "";
}

void _exporter_base::createStyledItemInstance(OwlInstance iOwlInstance, SdaiInstance iSdaiInstance)
{
	assert(iOwlInstance != 0);
	assert(iSdaiInstance != 0);

	// material
	OwlInstance* piMaterials = nullptr;
	int64_t iMaterialsCount = 0;
	GetObjectProperty(
		iOwlInstance,
		GetPropertyByName(getSite()->getOwlModel(), "material"),
		&piMaterials,
		&iMaterialsCount);

	assert(iMaterialsCount == 1);

	OwlInstance iMaterialInstance = piMaterials[0];

	if (hasObjectProperty(iMaterialInstance, "textures"))
	{
		m_pSite->logWarn("Textures are not supported.");

		createDefaultStyledItemInstance(iSdaiInstance);

		return;
	}

	string strTag = getTag(iMaterialInstance);
	if (strTag == "Default Material")
	{
		createDefaultStyledItemInstance(iSdaiInstance);

		return;
	}

	// color
	OwlInstance* piInstances = nullptr;
	int64_t iInstancesCount = 0;
	GetObjectProperty(
		iMaterialInstance,
		GetPropertyByName(getSite()->getOwlModel(), "color"),
		&piInstances,
		&iInstancesCount);

	assert(iInstancesCount == 1);

	OwlInstance iColorInstance = piInstances[0];

	// transparency
	double* pdValues = nullptr;
	int64_t iValuesCount = 0;
	GetDatatypeProperty(
		iColorInstance,
		GetPropertyByName(getSite()->getOwlModel(), "transparency"),
		(void**)&pdValues,
		&iValuesCount);

	double dTransparency = 0.;
	if (iValuesCount == 1)
	{
		dTransparency = 1. - pdValues[0];
	}

	// diffuse
	piInstances = nullptr;
	iInstancesCount = 0;
	GetObjectProperty(
		iColorInstance,
		GetPropertyByName(getSite()->getOwlModel(), "diffuse"),
		&piInstances,
		&iInstancesCount);

	assert(iInstancesCount == 1);

	OwlInstance iDiffuseColorComponentInstance = piInstances[0];

	// R
	double* pdRValue = nullptr;
	iValuesCount = 0;
	GetDatatypeProperty(
		iDiffuseColorComponentInstance,
		GetPropertyByName(getSite()->getOwlModel(), "R"),
		(void**)&pdRValue,
		&iValuesCount);

	assert(iValuesCount == 1);

	// G
	double* pdGValue = nullptr;
	iValuesCount = 0;
	GetDatatypeProperty(
		iDiffuseColorComponentInstance,
		GetPropertyByName(getSite()->getOwlModel(), "G"),
		(void**)&pdGValue,
		&iValuesCount);

	assert(iValuesCount == 1);

	// B
	double* pdBValue = nullptr;
	iValuesCount = 0;
	GetDatatypeProperty(
		iDiffuseColorComponentInstance,
		GetPropertyByName(getSite()->getOwlModel(), "B"),
		(void**)&pdBValue,
		&iValuesCount);

	assert(iValuesCount == 1);

	createStyledItemInstance(iSdaiInstance, pdRValue[0], pdGValue[0], pdBValue[0], dTransparency);
}

void _exporter_base::createStyledItemInstance(SdaiInstance iSdaiInstance, double dR, double dG, double dB, double dTransparency)
{
	assert(iSdaiInstance != 0);

	SdaiInstance iStyledItemInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcStyledItem");
	assert(iStyledItemInstance != 0);

	sdaiPutAttrBN(iStyledItemInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iStyledItemInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	SdaiAggr pStyles = sdaiCreateAggrBN(iStyledItemInstance, "Styles");
	assert(pStyles != nullptr);

	SdaiInstance iPresentationStyleAssignmentInstance = buildPresentationStyleAssignmentInstance();
	sdaiAppend(pStyles, sdaiINSTANCE, (void*)iPresentationStyleAssignmentInstance);

	pStyles = sdaiCreateAggrBN(iPresentationStyleAssignmentInstance, "Styles");
	assert(pStyles != nullptr);

	SdaiInstance iSurfaceStyleInstance = buildSurfaceStyleInstance();
	sdaiPutAttrBN(iSurfaceStyleInstance, "Side", sdaiENUM, "BOTH");
	sdaiAppend(pStyles, sdaiINSTANCE, (void*)iSurfaceStyleInstance);

	pStyles = sdaiCreateAggrBN(iSurfaceStyleInstance, "Styles");
	assert(pStyles != nullptr);

	SdaiInstance iSurfaceStyleRenderingInstance = buildSurfaceStyleRenderingInstance();
	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "ReflectanceMethod", sdaiENUM, "NOTDEFINED");
	sdaiAppend(pStyles, sdaiINSTANCE, (void*)iSurfaceStyleRenderingInstance);

	SdaiInstance iColorRgbInstance = buildColorRgbInstance(dR, dG, dB);
	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "SurfaceColour", sdaiINSTANCE, (void*)iColorRgbInstance);
	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "Transparency", sdaiREAL, &dTransparency);

	sdaiPutAttrBN(iStyledItemInstance, "Item", sdaiINSTANCE, (void*)iSdaiInstance);
}

void _exporter_base::createStyledItemInstance(SdaiInstance iSdaiInstance, SdaiInstance iColorRgbInstance, double dTransparency)
{
	assert(iSdaiInstance != 0);
	assert(iColorRgbInstance != 0);

	SdaiInstance iStyledItemInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcStyledItem");
	assert(iStyledItemInstance != 0);

	sdaiPutAttrBN(iStyledItemInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iStyledItemInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	SdaiAggr pStyles = sdaiCreateAggrBN(iStyledItemInstance, "Styles");
	assert(pStyles != nullptr);

	SdaiInstance iPresentationStyleAssignmentInstance = buildPresentationStyleAssignmentInstance();
	sdaiAppend(pStyles, sdaiINSTANCE, (void*)iPresentationStyleAssignmentInstance);

	pStyles = sdaiCreateAggrBN(iPresentationStyleAssignmentInstance, "Styles");
	assert(pStyles != nullptr);

	SdaiInstance iSurfaceStyleInstance = buildSurfaceStyleInstance();
	sdaiPutAttrBN(iSurfaceStyleInstance, "Side", sdaiENUM, "BOTH");
	sdaiAppend(pStyles, sdaiINSTANCE, (void*)iSurfaceStyleInstance);

	pStyles = sdaiCreateAggrBN(iSurfaceStyleInstance, "Styles");
	assert(pStyles != nullptr);

	SdaiInstance iSurfaceStyleRenderingInstance = buildSurfaceStyleRenderingInstance();
	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "ReflectanceMethod", sdaiENUM, "NOTDEFINED");
	sdaiAppend(pStyles, sdaiINSTANCE, (void*)iSurfaceStyleRenderingInstance);

	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "SurfaceColour", sdaiINSTANCE, (void*)iColorRgbInstance);
	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "Transparency", sdaiREAL, &dTransparency);

	sdaiPutAttrBN(iStyledItemInstance, "Item", sdaiINSTANCE, (void*)iSdaiInstance);
}

SdaiInstance _exporter_base::buildPresentationStyleAssignmentInstance()
{
	SdaiInstance iPresentationStyleAssignmentInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcPresentationStyleAssignment");
	assert(iPresentationStyleAssignmentInstance != 0);

	sdaiPutAttrBN(iPresentationStyleAssignmentInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iPresentationStyleAssignmentInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	return iPresentationStyleAssignmentInstance;
}

SdaiInstance _exporter_base::buildSurfaceStyleInstance()
{
	SdaiInstance iSurfaceStyleInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcSurfaceStyle");
	assert(iSurfaceStyleInstance != 0);

	sdaiPutAttrBN(iSurfaceStyleInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iSurfaceStyleInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	return iSurfaceStyleInstance;
}

SdaiInstance _exporter_base::buildSurfaceStyleRenderingInstance()
{
	SdaiInstance iSurfaceStyleRenderingInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcSurfaceStyleRendering");
	assert(iSurfaceStyleRenderingInstance != 0);

	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iSurfaceStyleRenderingInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	return iSurfaceStyleRenderingInstance;
}

SdaiInstance _exporter_base::buildColorRgbInstance(double dR, double dG, double dB)
{
	SdaiInstance iColorRgbInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcColourRgb");
	assert(iColorRgbInstance != 0);

	sdaiPutAttrBN(iColorRgbInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iColorRgbInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iColorRgbInstance, "Name", sdaiSTRING, "Color");
	sdaiPutAttrBN(iColorRgbInstance, "Red", sdaiREAL, &dR);
	sdaiPutAttrBN(iColorRgbInstance, "Green", sdaiREAL, &dG);
	sdaiPutAttrBN(iColorRgbInstance, "Blue", sdaiREAL, &dB);

	return iColorRgbInstance;
}

SdaiInstance _exporter_base::buildPropertySet(char* szName, SdaiAggr& pHasProperties)
{
	SdaiInstance iPropertySetInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcPropertySet");
	assert(iPropertySetInstance != 0);

	sdaiPutAttrBN(iPropertySetInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iPropertySetInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());
	sdaiPutAttrBN(iPropertySetInstance, "Name", sdaiSTRING, szName);

	pHasProperties = sdaiCreateAggrBN(iPropertySetInstance, "HasProperties");
	assert(pHasProperties != nullptr);

	return iPropertySetInstance;
}

SdaiInstance _exporter_base::buildRelDefinesByProperties(SdaiInstance iRelatedObject, SdaiInstance iRelatingPropertyDefinition)
{
	assert(iRelatedObject != 0);
	assert(iRelatingPropertyDefinition != 0);

	SdaiInstance iRelDefinesByPropertiesInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcRelDefinesByProperties");
	assert(iRelDefinesByPropertiesInstance != 0);

	sdaiPutAttrBN(iRelDefinesByPropertiesInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iRelDefinesByPropertiesInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	SdaiAggr pRelatedObjects = sdaiCreateAggrBN(iRelDefinesByPropertiesInstance, "RelatedObjects");
	assert(pRelatedObjects != 0);

	sdaiAppend(pRelatedObjects, sdaiINSTANCE, (void*)iRelatedObject);
	sdaiPutAttrBN(iRelDefinesByPropertiesInstance, "RelatingPropertyDefinition", sdaiINSTANCE, (void*)iRelatingPropertyDefinition);

	return iRelDefinesByPropertiesInstance;
}

SdaiInstance _exporter_base::buildPropertySingleValueText(
	const char* szName,
	const char* szDescription,
	const char* szNominalValue,
	const char* szTypePath)
{
	SdaiInstance iPropertySingleValueInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcPropertySingleValue");
	assert(iPropertySingleValueInstance != 0);

	sdaiPutAttrBN(iPropertySingleValueInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iPropertySingleValueInstance, "Description", sdaiSTRING, szDescription);

	SdaiADB pNominalValueADB = sdaiCreateADB(sdaiSTRING, szNominalValue);
	assert(pNominalValueADB != nullptr);

	sdaiPutADBTypePath(pNominalValueADB, 1, szTypePath);
	sdaiPutAttrBN(iPropertySingleValueInstance, "NominalValue", sdaiADB, (void*)pNominalValueADB);

	return iPropertySingleValueInstance;
}

SdaiInstance _exporter_base::buildPropertySingleValueReal(
	const char* szName,
	const char* szDescription,
	double dNominalValue,
	const char* szTypePath)
{
	SdaiInstance iPropertySingleValueInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcPropertySingleValue");
	assert(iPropertySingleValueInstance != 0);

	sdaiPutAttrBN(iPropertySingleValueInstance, "Name", sdaiSTRING, szName);
	sdaiPutAttrBN(iPropertySingleValueInstance, "Description", sdaiSTRING, szDescription);

	SdaiADB pNominalValueADB = sdaiCreateADB(sdaiREAL, (void*)&dNominalValue);
	assert(pNominalValueADB != nullptr);

	sdaiPutADBTypePath(pNominalValueADB, 1, szTypePath);
	sdaiPutAttrBN(iPropertySingleValueInstance, "NominalValue", sdaiADB, (void*)pNominalValueADB);

	return iPropertySingleValueInstance;
}

SdaiInstance _exporter_base::buildMaterial()
{
	SdaiInstance iMaterialInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcMaterial");
	assert(iMaterialInstance != 0);

	sdaiPutAttrBN(iMaterialInstance, "Name", sdaiSTRING, (void*)"Material");

	return iMaterialInstance;
}

SdaiInstance _exporter_base::buildMaterialLayer(double dThickness)
{
	SdaiInstance iMaterialLayerInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcMaterialLayer");
	assert(iMaterialLayerInstance != 0);

	sdaiPutAttrBN(iMaterialLayerInstance, "Material", sdaiINSTANCE, (void*)buildMaterial());
	sdaiPutAttrBN(iMaterialLayerInstance, "LayerThickness", sdaiREAL, &dThickness);

	return iMaterialLayerInstance;
}

SdaiInstance _exporter_base::buildMaterialLayerSet(double dThickness)
{
	SdaiInstance iMaterialLayerSetInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcMaterialLayerSet");
	assert(iMaterialLayerSetInstance != 0);

	SdaiAggr pMaterialLayers = sdaiCreateAggrBN(iMaterialLayerSetInstance, "MaterialLayers");
	assert(pMaterialLayers != nullptr);

	sdaiAppend(pMaterialLayers, sdaiINSTANCE, (void*)buildMaterialLayer(dThickness));

	return iMaterialLayerSetInstance;
}

SdaiInstance _exporter_base::buildMaterialLayerSetUsage(double dThickness)
{
	double dOffsetFromReferenceLine = -dThickness / 2.;

	SdaiInstance iMaterialLayerSetUsageInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcMaterialLayerSetUsage");
	assert(iMaterialLayerSetUsageInstance != 0);

	sdaiPutAttrBN(iMaterialLayerSetUsageInstance, "ForLayerSet", sdaiINSTANCE, (void*)buildMaterialLayerSet(dThickness));
	sdaiPutAttrBN(iMaterialLayerSetUsageInstance, "LayerSetDirection", sdaiENUM, "AXIS2");
	sdaiPutAttrBN(iMaterialLayerSetUsageInstance, "DirectionSense", sdaiENUM, "POSITIVE");
	sdaiPutAttrBN(iMaterialLayerSetUsageInstance, "OffsetFromReferenceLine", sdaiREAL, &dOffsetFromReferenceLine);

	return iMaterialLayerSetUsageInstance;
}

SdaiInstance _exporter_base::buildRelAssociatesMaterial(SdaiInstance iBuildingElementInstance, double dThickness)
{
	assert(iBuildingElementInstance != 0);

	SdaiInstance iRelAssociatesMaterialInstance = sdaiCreateInstanceBN(m_iSdaiModel, "IfcRelAssociatesMaterial");
	assert(iRelAssociatesMaterialInstance != 0);

	sdaiPutAttrBN(iRelAssociatesMaterialInstance, "GlobalId", sdaiSTRING, (void*)_guid::createGlobalId().c_str());
	sdaiPutAttrBN(iRelAssociatesMaterialInstance, "OwnerHistory", sdaiINSTANCE, (void*)getOwnerHistoryInstance());

	SdaiAggr pRelatedObjects = sdaiCreateAggrBN(iRelAssociatesMaterialInstance, "RelatedObjects");
	assert(pRelatedObjects != nullptr);

	sdaiAppend(pRelatedObjects, sdaiINSTANCE, (void*)iBuildingElementInstance);
	sdaiPutAttrBN(iRelAssociatesMaterialInstance, "RelatingMaterial", sdaiINSTANCE, (void*)buildMaterialLayerSetUsage(dThickness));

	return iRelAssociatesMaterialInstance;
}

string _exporter_base::getTag(OwlInstance iInstance) const
{
	assert(iInstance != 0);

	SetCharacterSerialization(getSite()->getOwlModel(), 0, 0, false);

	wchar_t** szValue = nullptr;
	int64_t iValuesCount = 0;
	GetDatatypeProperty(iInstance, m_iTagProperty, (void**)&szValue, &iValuesCount);
	assert(iValuesCount == 1);

	SetCharacterSerialization(getSite()->getOwlModel(), 0, 0, true);

	return (LPCSTR)CW2A(szValue[0]);
}

string _exporter_base::getStringAttributeValue(OwlInstance iInstance, const string& strAttributeName) const
{
	assert(iInstance != 0);
	assert(!strAttributeName.empty());

	int64_t iPropertyInstance = GetInstancePropertyByIterator(iInstance, 0);
	while (iPropertyInstance != 0)
	{
		char* szPropertyName = nullptr;
		GetNameOfProperty(iPropertyInstance, &szPropertyName);

		string strPropertyName = szPropertyName;
		if (strPropertyName == "attr:str:" + strAttributeName)
		{
			assert(GetPropertyType(iPropertyInstance) == DATATYPEPROPERTY_TYPE_WCHAR_T_ARRAY);

			SetCharacterSerialization(getSite()->getOwlModel(), 0, 0, false);

			wchar_t** szValue = nullptr;
			int64_t iValuesCount = 0;
			GetDatatypeProperty(iInstance,
				iPropertyInstance,
				(void**)&szValue, &iValuesCount);
			assert(iValuesCount == 1);

			auto iLength = std::char_traits<char16_t>::length((char16_t*)*szValue);

			u16string strValueU16;
			strValueU16.resize(iLength);
			memcpy((void*)strValueU16.data(), szValue[0], iLength * sizeof(char16_t));

			SetCharacterSerialization(getSite()->getOwlModel(), 0, 0, true);

			return To_UTF8(strValueU16);
		} // if (strPropertyName == ...

		iPropertyInstance = GetInstancePropertyByIterator(iInstance, iPropertyInstance);
	} // while (iPropertyInstance != 0)

	return "";
}

OwlInstance* _exporter_base::getObjectProperty(OwlInstance iInstance, const string& strPropertyName, int64_t& iInstancesCount) const
{
	assert(iInstance != 0);
	assert(!strPropertyName.empty());

	iInstancesCount = 0;

	RdfProperty iProperty = GetPropertyByName(getSite()->getOwlModel(), strPropertyName.c_str());
	if (iProperty == 0)
	{
		return nullptr;
	}

	OwlInstance* piInstances = nullptr;
	GetObjectProperty(
		iInstance,
		iProperty,
		&piInstances,
		&iInstancesCount);

	if (iInstancesCount == 0)
	{
		return nullptr;
	}

	return piInstances;
}

bool _exporter_base::hasObjectProperty(OwlInstance iInstance, const string& strPropertyName)
{
	int64_t iInstancesCount = 0;

	return getObjectProperty(iInstance, strPropertyName, iInstancesCount) != nullptr;
}

void _exporter_base::getPosValues(const string& strContent, vector<double>& vecValues) const
{
	assert(!strContent.empty());

	vector<string> vecStrValues;
	_string::split(strContent, " ", vecStrValues);

	if (vecStrValues.size() > 3)
	{
		assert(false);
	}
	else if (vecStrValues.size() < 3)
	{
		vecStrValues.resize(3, "0"); // Z
	}

	for (auto strValue : vecStrValues)
	{
		vecValues.push_back(atof(strValue.c_str()));
	}
}

void _exporter_base::getPosValuesW(const wstring& strContent, vector<double>& vecValues) const
{
	assert(!strContent.empty());

	getPosValues((LPCSTR)CW2A(strContent.c_str()), vecValues);	
}

// ************************************************************************************************
_citygml_exporter::_citygml_exporter(_gml2ifc_exporter* pSite)
	: _exporter_base(pSite)
	, m_fXmin(FLT_MAX)
	, m_fXmax(-FLT_MAX)
	, m_fYmin(FLT_MAX)
	, m_fYmax(-FLT_MAX)
	, m_fZmin(FLT_MAX)
	, m_fZmax(-FLT_MAX)
	, m_iCollectionClass(0)
	, m_iTransformationClass(0)
	, m_mapInstanceDefaultState()
	, m_mapMappedItems()
	, m_iCityModelClass(0)
	, m_iBoundingShapeClass(0)
	, m_iEnvelopeClass(0)
	, m_iModelEnvelopeInstance(0)
	, m_mapBuildingEnvelopeInstance()
	, m_iCityObjectGroupMemberClass(0)
	, m_iGeometryMemberClass(0)
	, m_iBuildingClass(0)
	, m_iWallSurfaceClass(0)
	, m_iRoofSurfaceClass(0)
	, m_iDoorClass(0)
	, m_iWindowClass(0)
	, m_mapBuildings()
	, m_mapBuildingElements()
	, m_iCadastralParcelClass(0)
	, m_iReferencePointIndicatorClass(0)
	, m_iVegetationObjectClass(0)
	, m_iWaterObjectClass(0)
	, m_iBridgeObjectClass(0)
	, m_iTunnelObjectClass(0)
	, m_iTransportationObjectClass(0)
	, m_iFurnitureObjectClass(0)
	, m_iReliefObjectClass(0)
	, m_iLandUseClass(0)
	, m_iTrafficSpaceClass(0)
	, m_iTrafficAreaClass(0)
	, m_iThingClass(0)
	, m_mapFeatures()
	, m_mapFeatureElements()
	, m_iCurrentOwlBuildingElementInstance(0)	
	, m_iDefaultWallSurfaceColorRgbInstance(0)
	, m_iDefaultRoofSurfaceColorRgbInstance(0)
	, m_iDefaultDoorColorRgbInstance(0)
	, m_iDefaultWindowColorRgbInstance(0)
	, m_iDefaultColorRgbInstance(0)
{
	// Geometry Kernel
	m_iCollectionClass = GetClassByName(getSite()->getOwlModel(), "Collection");
	m_iTransformationClass = GetClassByName(getSite()->getOwlModel(), "Transformation");	

	// CRS
	m_iCityModelClass = GetClassByName(getSite()->getOwlModel(), "class:CityModelType");
	m_iBoundingShapeClass = GetClassByName(getSite()->getOwlModel(), "class:BoundingShapeType");
	m_iEnvelopeClass = GetClassByName(getSite()->getOwlModel(), "class:EnvelopeType");

	// CityObjectGroup
	m_iCityObjectGroupMemberClass = GetClassByName(getSite()->getOwlModel(), "class:CityObjectGroupMemberType");

	// relativeGMLGeometry
	m_iGeometryMemberClass = GetClassByName(getSite()->getOwlModel(), "class:geometryMember");

	// Building
	m_iBuildingClass = GetClassByName(getSite()->getOwlModel(), "class:Building");
	m_iWallSurfaceClass = GetClassByName(getSite()->getOwlModel(), "class:WallSurface");
	m_iRoofSurfaceClass = GetClassByName(getSite()->getOwlModel(), "class:RoofSurface");
	m_iDoorClass = GetClassByName(getSite()->getOwlModel(), "class:Door");
	m_iWindowClass = GetClassByName(getSite()->getOwlModel(), "class:Window");

	// Parcel
	m_iCadastralParcelClass = GetClassByName(getSite()->getOwlModel(), "class:CadastralParcelType");
	m_iReferencePointIndicatorClass = GetClassByName(getSite()->getOwlModel(), "ReferencePointIndicator");

	// Feature
	m_iVegetationObjectClass = GetClassByName(getSite()->getOwlModel(), "class:_VegetationObject");
	m_iWaterObjectClass = GetClassByName(getSite()->getOwlModel(), "class:_WaterObject");
	m_iBridgeObjectClass = GetClassByName(getSite()->getOwlModel(), "class:_AbstractBridge");
	m_iTunnelObjectClass = GetClassByName(getSite()->getOwlModel(), "class:_AbstractTunnel");
	m_iTransportationObjectClass = GetClassByName(getSite()->getOwlModel(), "class:_TransportationObject");
	m_iFurnitureObjectClass = GetClassByName(getSite()->getOwlModel(), "class:CityFurniture");
	m_iReliefObjectClass = GetClassByName(getSite()->getOwlModel(), "class:_ReliefComponent");
	m_iLandUseClass = GetClassByName(getSite()->getOwlModel(), "class:LandUse");
	m_iTrafficSpaceClass = GetClassByName(getSite()->getOwlModel(), "class:trafficSpace");
	m_iTrafficAreaClass = GetClassByName(getSite()->getOwlModel(), "class:TrafficArea");
	m_iThingClass = GetClassByName(getSite()->getOwlModel(), "class:Thing");
}

/*virtual*/ _citygml_exporter::~_citygml_exporter()
{}

/*virtual*/ int _citygml_exporter::retrieveSRSData(OwlInstance iRootInstance) /*override*/
{
	assert(iRootInstance != 0);

	collectSRSData(iRootInstance);

	int iTransformationsCount = 0;
	if (!m_mapBuildingEnvelopeInstance.empty())
	{
		for (auto itBuildingEnvelopeInstance : m_mapBuildingEnvelopeInstance)
		{
			if (transformEnvelopeSRSDataAsync(itBuildingEnvelopeInstance.second))
			{
				iTransformationsCount++;
			}
		}
	}
	else if (m_iModelEnvelopeInstance != 0)
	{
		if (transformEnvelopeSRSDataAsync(m_iModelEnvelopeInstance))
		{
			iTransformationsCount++;
		}
	}

	return iTransformationsCount;
}

/*virtual*/ void _citygml_exporter::preProcessing() /*override*/
{
	getInstancesDefaultState();

	OwlInstance iInstance = GetInstancesByIterator(getSite()->getOwlModel(), 0);
	while (iInstance != 0)
	{		
		_vector3d vecBBMin;
		_vector3d vecBBMax;
		if (m_mapInstanceDefaultState.at(iInstance) &&
			GetInstanceGeometryClass(iInstance) &&
			GetBoundingBox(
				iInstance,
				(double*)&vecBBMin,
				(double*)&vecBBMax))
		{
			m_fXmin = (float)fmin(m_fXmin, vecBBMin.x);
			m_fXmax = (float)fmax(m_fXmax, vecBBMax.x);
			m_fYmin = (float)fmin(m_fYmin, vecBBMin.y);
			m_fYmax = (float)fmax(m_fYmax, vecBBMax.y);
			m_fZmin = (float)fmin(m_fZmin, vecBBMin.z);
			m_fZmax = (float)fmax(m_fZmax, vecBBMax.z);
		}

		iInstance = GetInstancesByIterator(getSite()->getOwlModel(), iInstance);
	} // while (iInstance != 0)
}

/*virtual*/ void _citygml_exporter::executeCore(OwlInstance iRootInstance, const wstring& strOuputFile)
{
	assert(iRootInstance != 0);
	assert(!strOuputFile.empty());

	m_iModelEnvelopeInstance = 0;
	m_mapBuildingEnvelopeInstance.clear();

	m_mapBuildings.clear();
	m_mapBuildingElements.clear();

	m_mapFeatures.clear();
	m_mapFeatureElements.clear();

	collectSRSData(iRootInstance);

	createIfcModel(L"IFC4");

	if (!m_mapBuildingEnvelopeInstance.empty())
	{
		printf("TODO: not implemented.\n");
		assert(false); //#todo
	}
	else if (m_iModelEnvelopeInstance != 0)
	{
		string strTag = getTag(iRootInstance);

		OwlClass iInstanceClass = GetInstanceClass(iRootInstance);
		assert(iInstanceClass != 0);

		char* szClassName = nullptr;
		GetNameOfClass(iInstanceClass, &szClassName);
		assert(szClassName != nullptr);

		_matrix mtxIdentity;
		SdaiInstance iSiteInstancePlacement = 0;
		SdaiInstance iSiteInstance = buildSiteInstance(
			strTag.c_str(),
			szClassName,
			&mtxIdentity,
			iSiteInstancePlacement);
		assert(iSiteInstancePlacement != 0);

		string strEPSGCode;
		vector<double> vecLowerCorner;
		vector<double> vecUpperCorner;
		if (retrieveEnvelopeSRSData(m_iModelEnvelopeInstance, strEPSGCode, vecLowerCorner, vecUpperCorner))
		{
			string strCoordinates;
			if (getSite()->getWGS84(
				atoi(strEPSGCode.c_str()),
				(float)(vecLowerCorner[0] + vecUpperCorner[0]) / 2.f,
				(float)(vecLowerCorner[1] + vecUpperCorner[1]) / 2.f,
				(float)(vecLowerCorner[2] + vecUpperCorner[2]) / 2.f,
				strCoordinates))
			{
				vector<double> vecCoordinates;
				getPosValues(strCoordinates, vecCoordinates);

				double dLatitude = vecCoordinates[0];
				double dLongitude = vecCoordinates[1];

				SdaiAggr pRefLatitude = sdaiCreateAggrBN(iSiteInstance, "RefLatitude");
				assert(pRefLatitude != nullptr);

				/*
				  c[1] :=    a;                                           -- -50
				  c[2] :=   (a - c[1]) * 60;                              -- -58
				  c[3] :=  ((a - c[1]) * 60 - c[2]) * 60;                 -- -33
				  c[4] := (((a - c[1]) * 60 - c[2]) * 60 - c[3]) * 1.e6;  -- -110400
				*/
				
				int64_t iRefLatitude1 = (int64_t)dLatitude;
				int64_t iRefLatitude2 = (int64_t)((dLatitude - iRefLatitude1) * 60.);
				int64_t iRefLatitude3 = (int64_t)(((dLatitude - iRefLatitude1) * 60. - iRefLatitude2) * 60.);
				int64_t iRefLatitude4 = 0;
				sdaiAppend(pRefLatitude, sdaiINTEGER, &iRefLatitude1);
				sdaiAppend(pRefLatitude, sdaiINTEGER, &iRefLatitude2);
				sdaiAppend(pRefLatitude, sdaiINTEGER, &iRefLatitude3);
				sdaiAppend(pRefLatitude, sdaiINTEGER, &iRefLatitude4);

				SdaiAggr pRefLongitude = sdaiCreateAggrBN(iSiteInstance, "RefLongitude");
				assert(pRefLongitude != nullptr);

				int64_t iRefLongitude1 = (int64_t)dLongitude;
				int64_t iRefLongitude2 = (int64_t)((dLongitude - iRefLongitude1) * 60.);
				int64_t iRefLongitude3 = (int64_t)(((dLongitude - iRefLongitude1) * 60. - iRefLongitude2) * 60.);
				int64_t iRefLongitude4 = 0;
				sdaiAppend(pRefLongitude, sdaiINTEGER, &iRefLongitude1);
				sdaiAppend(pRefLongitude, sdaiINTEGER, &iRefLongitude2);
				sdaiAppend(pRefLongitude, sdaiINTEGER, &iRefLongitude3);
				sdaiAppend(pRefLongitude, sdaiINTEGER, &iRefLongitude4);

				double dRefElevation = vecLowerCorner[1];
				sdaiPutAttrBN(iSiteInstance, "RefElevation", sdaiREAL, &dRefElevation);
			} // if (getSite()->getWGS84( ...

			printf("*** %s\n", strCoordinates.c_str());
		} // if (retrieveEnvelopeSRSData( ...

		createProperties(iRootInstance, iSiteInstance);

		buildRelAggregatesInstance(
			"ProjectContainer",
			"ProjectContainer for Sites",
			getProjectInstance(),
			vector<SdaiInstance>{iSiteInstance});

		createBuildings(iSiteInstance, iSiteInstancePlacement);
		createFeatures(iSiteInstance, iSiteInstancePlacement);
	} // else if (m_iModelEnvelopeInstance != 0)

	saveIfcFile(strOuputFile.c_str());
}

/*virtual*/ void _citygml_exporter::postProcessing() /*override*/
{
}

/*virtual*/ OwlInstance _citygml_exporter::getModelEnvelopeInstance() /*override*/
{
	return m_iModelEnvelopeInstance;
}

/*virtual*/ void _citygml_exporter::createDefaultStyledItemInstance(SdaiInstance iSdaiInstance) /*override*/
{
	assert(m_iCurrentOwlBuildingElementInstance != 0);
	assert(iSdaiInstance != 0);

	OwlClass iInstanceClass = GetInstanceClass(m_iCurrentOwlBuildingElementInstance);
	assert(iInstanceClass != 0);

	if (isWallSurfaceClass(iInstanceClass))
	{
		if (m_iDefaultWallSurfaceColorRgbInstance == 0)
		{
			m_iDefaultWallSurfaceColorRgbInstance = buildColorRgbInstance(128. / 255., 128. / 255., 128. / 255.);
		}

		createStyledItemInstance(iSdaiInstance, m_iDefaultWallSurfaceColorRgbInstance, 0.);
	}
	else if (isRoofSurfaceClass(iInstanceClass))
	{
		if (m_iDefaultRoofSurfaceColorRgbInstance == 0)
		{
			m_iDefaultRoofSurfaceColorRgbInstance = buildColorRgbInstance(139. / 255., 69. / 255., 19. / 255.);
		}

		createStyledItemInstance(iSdaiInstance, m_iDefaultRoofSurfaceColorRgbInstance, 0.);
	}
	else if (isDoorClass(iInstanceClass))
	{
		if (m_iDefaultDoorColorRgbInstance == 0)
		{
			m_iDefaultDoorColorRgbInstance = buildColorRgbInstance(139. / 255., 139. / 255., 139. / 255.);
		}

		createStyledItemInstance(iSdaiInstance, m_iDefaultDoorColorRgbInstance, 0.);
	}
	else if (isWindowClass(iInstanceClass))
	{
		if (m_iDefaultWindowColorRgbInstance == 0)
		{
			m_iDefaultWindowColorRgbInstance = buildColorRgbInstance(25. / 255., 25. / 255., 25. / 255.);
		}

		createStyledItemInstance(iSdaiInstance, m_iDefaultWindowColorRgbInstance, 0.95);
	}
	else
	{
		if (m_iDefaultColorRgbInstance == 0)
		{
			m_iDefaultColorRgbInstance = buildColorRgbInstance(0., 0., 1.);
		}

		createStyledItemInstance(iSdaiInstance, m_iDefaultColorRgbInstance, 0.75);
	}	
}

void _citygml_exporter::createBuildings(SdaiInstance iSiteInstance, SdaiInstance iSiteInstancePlacement)
{
	assert(iSiteInstance != 0);
	assert(iSiteInstancePlacement != 0);

	OwlClass iSchemasClass = GetClassByName(getSite()->getOwlModel(), "class:Schemas");
	assert(iSchemasClass != 0);

	OwlInstance iInstance = GetInstancesByIterator(getSite()->getOwlModel(), 0);
	while (iInstance != 0)
	{
		OwlClass iInstanceClass = GetInstanceClass(iInstance);
		assert(iInstanceClass != 0);

		if (GetInstanceInverseReferencesByIterator(iInstance, 0) == 0)
		{
			if (iInstanceClass != iSchemasClass)
			{
				if (isBuildingClass(iInstanceClass))
				{
					if (m_mapBuildings.find(iInstance) == m_mapBuildings.end())
					{
						m_mapBuildings[iInstance] = vector<OwlInstance>();

						searchForBuildingElements(iInstance, iInstance);
					}
					else
					{
						assert(false); // Internal error!
					}
				}
				else
				{
					createBuildingsRecursively(iInstance);
				}
			}
		} // if (GetInstanceInverseReferencesByIterator(iInstance, 0) == 0)

		iInstance = GetInstancesByIterator(getSite()->getOwlModel(), iInstance);
	} // while (iInstance != 0)
		
	if (m_mapBuildings.empty())
	{
		return;
	}
		
	_matrix mtxIdentity;
	vector<SdaiInstance> vecBuildingInstances;
	for (auto& itBuilding : m_mapBuildings)
	{
		string strTag = getTag(itBuilding.first);

		OwlClass iInstanceClass = GetInstanceClass(itBuilding.first);
		assert(iInstanceClass != 0);

		char* szClassName = nullptr;
		GetNameOfClass(iInstanceClass, &szClassName);
		assert(szClassName != nullptr);

		SdaiInstance iBuildingInstancePlacement = 0;
		SdaiInstance iBuildingInstance = buildBuildingInstance(
			strTag.c_str(),
			szClassName,
			&mtxIdentity, 
			iSiteInstancePlacement, 
			iBuildingInstancePlacement);
		assert(iBuildingInstance != 0);

		createProperties(itBuilding.first, iBuildingInstance);

		vecBuildingInstances.push_back(iBuildingInstance);

		// Proxy/Unknown Building Elements
		searchForProxyBuildingElements(itBuilding.first, itBuilding.first);

		if (itBuilding.second.empty())
		{
			continue;
		}
		
		vector<SdaiInstance> vecBuildingElementInstances;
		for (auto iOwlBuildingElementInstance : itBuilding.second)
		{
			m_iCurrentOwlBuildingElementInstance = iOwlBuildingElementInstance;

			auto itBuildingElement = m_mapBuildingElements.find(iOwlBuildingElementInstance);
			assert(itBuildingElement != m_mapBuildingElements.end());
			assert(!itBuildingElement->second.empty());

			vector<SdaiInstance> vecSdaiBuildingElementGeometryInstances;
			for (auto iOwlBuildingElementGeometryInstance : itBuildingElement->second)
			{
				vector<SdaiInstance> vecNewGeometryInstances;
				createGeometry(iOwlBuildingElementGeometryInstance, vecNewGeometryInstances, true);

				vecSdaiBuildingElementGeometryInstances.insert(
					vecSdaiBuildingElementGeometryInstances.end(),
					vecNewGeometryInstances.begin(),
					vecNewGeometryInstances.end());
			}

			if (vecSdaiBuildingElementGeometryInstances.empty())
			{
				// Not supported
				continue;
			}
			
			SdaiInstance iBuildingElementInstancePlacement = 0;
			SdaiInstance iSdaiBuildingElementInstance = buildBuildingElementInstance(
				itBuildingElement->first,
				&mtxIdentity,
				iBuildingInstancePlacement,
				iBuildingElementInstancePlacement,
				vecSdaiBuildingElementGeometryInstances);
			assert(iSdaiBuildingElementInstance != 0);

			createProperties(iOwlBuildingElementInstance, iSdaiBuildingElementInstance);

			vecBuildingElementInstances.push_back(iSdaiBuildingElementInstance);

			m_iCurrentOwlBuildingElementInstance = 0;
		} // for (auto iOwlBuildingElementInstance : ...

		SdaiInstance iBuildingStoreyInstancePlacement = 0;
		SdaiInstance iBuildingStoreyInstance = buildBuildingStoreyInstance(&mtxIdentity, iBuildingInstancePlacement, iBuildingStoreyInstancePlacement);
		assert(iBuildingStoreyInstance != 0);

		buildRelAggregatesInstance(
			"BuildingContainer", 
			"BuildingContainer for BuildigStories", 
			iBuildingInstance, 
			vector<SdaiInstance>{ iBuildingStoreyInstance });

		if (vecBuildingElementInstances.empty())
		{
			// Not supported
			continue;
		}		

		buildRelContainedInSpatialStructureInstance(
			"BuildingStoreyContainer", 
			"BuildingStoreyContainer for Building Elements", 
			iBuildingStoreyInstance, 
			vecBuildingElementInstances);
	} // for (auto& itBuilding : ...

	buildRelAggregatesInstance(
		"SiteContainer", 
		"SiteContainer For Buildings", 
		iSiteInstance, 
		vecBuildingInstances);
}

void _citygml_exporter::createBuildingsRecursively(OwlInstance iInstance)
{
	assert(iInstance != 0);

	RdfProperty iProperty = GetInstancePropertyByIterator(iInstance, 0);
	while (iProperty != 0)
	{
		if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)
		{
			int64_t iValuesCount = 0;
			OwlInstance* piValues = nullptr;
			GetObjectProperty(iInstance, iProperty, &piValues, &iValuesCount);

			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
			{
				if (piValues[iValue] == 0)
				{
					continue;
				}
					
				OwlClass iInstanceClass = GetInstanceClass(piValues[iValue]);
				assert(iInstanceClass != 0);

				if ((iInstanceClass == m_iCityObjectGroupMemberClass) || IsClassAncestor(iInstanceClass, m_iCityObjectGroupMemberClass))
				{
					continue; // Ignore
				}

				if (isBuildingClass(iInstanceClass))
				{
					if (m_mapBuildings.find(piValues[iValue]) == m_mapBuildings.end())
					{
						m_mapBuildings[piValues[iValue]] = vector<OwlInstance>();

						searchForBuildingElements(piValues[iValue], piValues[iValue]);
					}
					else
					{
						assert(false); // Internal error!
					}
				}
				else
				{
					createBuildingsRecursively(piValues[iValue]);
				}
			} // for (int64_t iValue = ...
		} // if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)

		iProperty = GetInstancePropertyByIterator(iInstance, iProperty);
	} // while (iProperty != 0)
}

void _citygml_exporter::searchForBuildingElements(OwlInstance iBuildingInstance, OwlInstance iInstance)
{
	assert(iBuildingInstance != 0);
	assert(iInstance != 0);

	RdfProperty iProperty = GetInstancePropertyByIterator(iInstance, 0);
	while (iProperty != 0)
	{
		if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)
		{
			int64_t iValuesCount = 0;
			OwlInstance* piValues = nullptr;
			GetObjectProperty(iInstance, iProperty, &piValues, &iValuesCount);

			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
			{
				if (piValues[iValue] == 0)
				{
					continue;
				}

				if (isBuildingElement(piValues[iValue]))
				{
					auto itBuilding = m_mapBuildings.find(iBuildingInstance);
					if (itBuilding != m_mapBuildings.end())
					{
						itBuilding->second.push_back(piValues[iValue]);
					}
					else
					{
						m_mapBuildings[iBuildingInstance] = vector<OwlInstance>{ piValues[iValue] };
					}

					searchForBuildingElementGeometry(piValues[iValue], piValues[iValue]);
				}

				searchForBuildingElements(iBuildingInstance, piValues[iValue]);
			} // for (int64_t iValue = ...
		} // if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)

		iProperty = GetInstancePropertyByIterator(iInstance, iProperty);
	} // while (iProperty != 0)
}

void _citygml_exporter::searchForProxyBuildingElements(OwlInstance iBuildingInstance, OwlInstance iInstance)
{
	assert(iBuildingInstance != 0);
	assert(iInstance != 0);

	RdfProperty iProperty = GetInstancePropertyByIterator(iInstance, 0);
	while (iProperty != 0)
	{
		if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)
		{
			int64_t iValuesCount = 0;
			OwlInstance* piValues = nullptr;
			GetObjectProperty(iInstance, iProperty, &piValues, &iValuesCount);

			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
			{
				if (piValues[iValue] == 0)
				{
					continue;
				}

				if (isBuildingElement(piValues[iValue]))
				{
					continue;
				}

				if (GetInstanceGeometryClass(piValues[iValue]) &&
					GetBoundingBox(piValues[iValue], nullptr, nullptr))
				{
					auto itBuilding = m_mapBuildings.find(iBuildingInstance);
					if (itBuilding != m_mapBuildings.end())
					{
						itBuilding->second.push_back(piValues[iValue]);
					}
					else
					{
						assert(false); // Internal error!
					}

					auto itBuildingElement = m_mapBuildingElements.find(piValues[iValue]);
					if (itBuildingElement == m_mapBuildingElements.end())
					{
						m_mapBuildingElements[piValues[iValue]] = vector<OwlInstance>{ piValues[iValue] };
					}
					else
					{
						OwlClass iChildInstanceClass = GetInstanceClass(piValues[iValue]);
						assert(iChildInstanceClass != 0);

						wchar_t* szClassName = nullptr;
						GetNameOfClassW(iChildInstanceClass, &szClassName);

						string strEvent = "Duplicated Geometry: '";
						strEvent += CW2A(szClassName);
						strEvent += "'";
						getSite()->logErr(strEvent);
					}
				}
				else
				{
					searchForProxyBuildingElements(iBuildingInstance, piValues[iValue]);
				}
			} // for (int64_t iValue = ...
		} // if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)

		iProperty = GetInstancePropertyByIterator(iInstance, iProperty);
	} // while (iProperty != 0)
}

void _citygml_exporter::searchForBuildingElementGeometry(OwlInstance iBuildingElementInstance, OwlInstance iInstance)
{
	assert(iBuildingElementInstance != 0);
	assert(iInstance != 0);

	RdfProperty iProperty = GetInstancePropertyByIterator(iInstance, 0);
	while (iProperty != 0)
	{
		if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)
		{
			int64_t iValuesCount = 0;
			OwlInstance* piValues = nullptr;
			GetObjectProperty(iInstance, iProperty, &piValues, &iValuesCount);

			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
			{
				if (piValues[iValue] == 0)
				{
					continue;
				}

				if (isBuildingElement(piValues[iValue]))
				{
					continue;
				}

				if (GetInstanceGeometryClass(piValues[iValue]) &&
					GetBoundingBox(piValues[iValue], nullptr, nullptr))
				{
					auto itBuildingElement = m_mapBuildingElements.find(iBuildingElementInstance);
					if (itBuildingElement != m_mapBuildingElements.end())
					{
						itBuildingElement->second.push_back(piValues[iValue]);
					}
					else
					{
						m_mapBuildingElements[iBuildingElementInstance] = vector<OwlInstance>{ piValues[iValue] };
					}
				}
				else
				{
					searchForBuildingElementGeometry(iBuildingElementInstance, piValues[iValue]);
				}
			} // for (int64_t iValue = ...
		} // if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)

		iProperty = GetInstancePropertyByIterator(iInstance, iProperty);
	} // while (iProperty != 0)
}

void _citygml_exporter::createFeatures(SdaiInstance iSiteInstance, SdaiInstance iSiteInstancePlacement)
{
	assert(iSiteInstance != 0);
	assert(iSiteInstancePlacement != 0);

	OwlClass iSchemasClass = GetClassByName(getSite()->getOwlModel(), "class:Schemas");
	assert(iSchemasClass != 0);

	OwlInstance iInstance = GetInstancesByIterator(getSite()->getOwlModel(), 0);
	while (iInstance != 0)
	{
		if (GetInstanceInverseReferencesByIterator(iInstance, 0) == 0)
		{
			OwlClass iInstanceClass = GetInstanceClass(iInstance);
			assert(iInstanceClass != 0);

			if (iInstanceClass != iSchemasClass)
			{
				if (isFeatureClass(iInstanceClass))
				{
					if (m_mapFeatures.find(iInstance) == m_mapFeatures.end())
					{
						m_mapFeatures[iInstance] = vector<OwlInstance>();

						searchForFeatureElements(iInstance, iInstance);
					}
					else
					{
						assert(false); // Internal error!
					}
				}
				else
				{
					createFeaturesRecursively(iInstance);
				}
			}
		} // if (GetInstanceInverseReferencesByIterator(iInstance, 0) == 0)

		iInstance = GetInstancesByIterator(getSite()->getOwlModel(), iInstance);
	} // while (iInstance != 0)

	if (m_mapFeatures.empty())
	{
		return;
	}

	_matrix mtxIdentity;
	vector<SdaiInstance> vecFeatureInstances;
	for (auto& itFeature : m_mapFeatures)
	{
		if (itFeature.second.empty())
		{
			continue;
		}

		// Geometry
		vector<SdaiInstance> vecSdaiFeatureElementGeometryInstances;
		for (auto iOwlFeatureElementInstance : itFeature.second)
		{
			m_iCurrentOwlBuildingElementInstance = iOwlFeatureElementInstance;

			auto itFeatureElement = m_mapFeatureElements.find(iOwlFeatureElementInstance);
			assert(itFeatureElement != m_mapFeatureElements.end());
			assert(!itFeatureElement->second.empty());

			for (auto iOwlFeatureElementGeometryInstance : itFeatureElement->second)
			{
				vector<SdaiInstance> vecNewGeometryInstances;
				createGeometry(iOwlFeatureElementGeometryInstance, vecNewGeometryInstances, true);
				
				vecSdaiFeatureElementGeometryInstances.insert(
					vecSdaiFeatureElementGeometryInstances.end(),
					vecNewGeometryInstances.begin(),
					vecNewGeometryInstances.end());
			}

			m_iCurrentOwlBuildingElementInstance = 0;
		} // for (auto iOwlFeatureElementInstance : ...

		if (vecSdaiFeatureElementGeometryInstances.empty())
		{
			// Not supported
			continue;
		}

		// Feature
		string strTag = getTag(itFeature.first);

		OwlClass iInstanceClass = GetInstanceClass(itFeature.first);
		assert(iInstanceClass != 0);

		char* szClassName = nullptr;
		GetNameOfClass(iInstanceClass, &szClassName);
		assert(szClassName != nullptr);

		SdaiInstance iFeatureInstancePlacement = 0;
		if (isTransportationObjectClass(iInstanceClass) ||
			isTrafficSpaceClass(iInstanceClass) ||
			isTrafficAreaClass(iInstanceClass) ||
			isBridgeObjectClass(iInstanceClass) ||
			isTunnelObjectClass(iInstanceClass))
		{			
			SdaiInstance iFeatureInstance = buildTransportElementInstance(
				strTag.c_str(),
				szClassName,
				&mtxIdentity,
				iSiteInstancePlacement,
				iFeatureInstancePlacement,
				vecSdaiFeatureElementGeometryInstances);
			assert(iFeatureInstance != 0);

			createProperties(itFeature.first, iFeatureInstance);

			vecFeatureInstances.push_back(iFeatureInstance);
		}
		else if (isReliefObjectClass(iInstanceClass) ||
			isLandUseClass(iInstanceClass) ||
			isWaterObjectClass(iInstanceClass) ||
			isVegetationObjectClass(iInstanceClass))
		{
			SdaiInstance iFeatureInstance = buildGeographicElementInstance(
				strTag.c_str(),
				szClassName,
				&mtxIdentity,
				iSiteInstancePlacement,
				iFeatureInstancePlacement,
				vecSdaiFeatureElementGeometryInstances);
			assert(iFeatureInstance != 0);

			createProperties(itFeature.first, iFeatureInstance);

			vecFeatureInstances.push_back(iFeatureInstance);
		}
		else if (isFurnitureObjectClass(iInstanceClass))
		{
			SdaiInstance iFeatureInstance = buildFurnitureObjectInstance(
				strTag.c_str(),
				szClassName,
				&mtxIdentity,
				iSiteInstancePlacement,
				iFeatureInstancePlacement,
				vecSdaiFeatureElementGeometryInstances);
			assert(iFeatureInstance != 0);

			createProperties(itFeature.first, iFeatureInstance);

			vecFeatureInstances.push_back(iFeatureInstance);
		}
		else
		{
			SdaiInstance iFeatureInstance = buildFeatureInstance(
				strTag.c_str(),
				szClassName,
				&mtxIdentity,
				iSiteInstancePlacement,
				iFeatureInstancePlacement,
				vecSdaiFeatureElementGeometryInstances);
			assert(iFeatureInstance != 0);

			createProperties(itFeature.first, iFeatureInstance);

			vecFeatureInstances.push_back(iFeatureInstance);
		}		
	} // for (auto& itFeature : ...

	buildRelAggregatesInstance(
		"SiteContainer",
		"SiteContainer For Features",
		iSiteInstance,
		vecFeatureInstances);
}

void _citygml_exporter::createFeaturesRecursively(OwlInstance iInstance)
{
	assert(iInstance != 0);

	RdfProperty iProperty = GetInstancePropertyByIterator(iInstance, 0);
	while (iProperty != 0)
	{
		if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)
		{
			int64_t iValuesCount = 0;
			OwlInstance* piValues = nullptr;
			GetObjectProperty(iInstance, iProperty, &piValues, &iValuesCount);

			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
			{
				if (piValues[iValue] == 0)
				{
					continue;
				}

				OwlClass iInstanceClass = GetInstanceClass(piValues[iValue]);
				assert(iInstanceClass != 0);

				if ((iInstanceClass == m_iCityObjectGroupMemberClass) || IsClassAncestor(iInstanceClass, m_iCityObjectGroupMemberClass))
				{					
					continue; // Ignore
				}

				if (isFeatureClass(iInstanceClass))
				{
					if (m_mapFeatures.find(piValues[iValue]) == m_mapFeatures.end())
					{
						m_mapFeatures[piValues[iValue]] = vector<OwlInstance>();

						searchForFeatureElements(piValues[iValue], piValues[iValue]);
					}
				}
				else
				{
					createFeaturesRecursively(piValues[iValue]);
				}
			} // for (int64_t iValue = ...
		} // if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)

		iProperty = GetInstancePropertyByIterator(iInstance, iProperty);
	} // while (iProperty != 0)
}

void _citygml_exporter::searchForFeatureElements(OwlInstance iFeatureInstance, OwlInstance iInstance)
{
	assert(iFeatureInstance != 0);
	assert(iInstance != 0);

	RdfProperty iProperty = GetInstancePropertyByIterator(iInstance, 0);
	while (iProperty != 0)
	{
		if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)
		{
			int64_t iValuesCount = 0;
			OwlInstance* piValues = nullptr;
			GetObjectProperty(iInstance, iProperty, &piValues, &iValuesCount);

			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
			{
				if (piValues[iValue] == 0)
				{
					continue;
				}

				if (GetInstanceGeometryClass(piValues[iValue]) &&
					GetBoundingBox(piValues[iValue], nullptr, nullptr))
				{
					auto itFeature = m_mapFeatures.find(iFeatureInstance);
					if (itFeature != m_mapFeatures.end())
					{
						itFeature->second.push_back(piValues[iValue]);
					}
					else
					{
						assert(false); // Internal error!
					}

					auto itFeatureElement = m_mapFeatureElements.find(piValues[iValue]);
					if (itFeatureElement == m_mapFeatureElements.end())
					{
						m_mapFeatureElements[piValues[iValue]] = vector<OwlInstance>{ piValues[iValue] };
					}
					else
					{
						OwlClass iChildInstanceClass = GetInstanceClass(piValues[iValue]);
						assert(iChildInstanceClass != 0);

						wchar_t* szClassName = nullptr;
						GetNameOfClassW(iChildInstanceClass, &szClassName);

						string strEvent = "Duplicated Geometry: '";
						strEvent += CW2A(szClassName);
						strEvent += "'";
						getSite()->logErr(strEvent);
					}
				}
				else
				{
					searchForFeatureElements(iFeatureInstance, piValues[iValue]);
				}
			} // for (int64_t iValue = ...
		} // if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)

		iProperty = GetInstancePropertyByIterator(iInstance, iProperty);
	} // while (iProperty != 0)
}

void _citygml_exporter::createGeometry(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	OwlClass iInstanceClass = GetInstanceClass(iInstance);
	assert(iInstanceClass != 0);

	if (iInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:MultiSurfaceType"))
	{
		createMultiSurface(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (iInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:SolidType"))
	{
		createSolid(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (iInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:CompositeSolidType"))
	{
		createCompositeSolid(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (iInstanceClass == GetClassByName(getSite()->getOwlModel(), "BoundaryRepresentation"))
	{
		createBoundaryRepresentation(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (iInstanceClass == GetClassByName(getSite()->getOwlModel(), "Point3D"))
	{
		//#todo
		//createPoint3D(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (iInstanceClass == GetClassByName(getSite()->getOwlModel(), "Point3DSet"))
	{
		//#todo
		//createPoint3DSet(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (iInstanceClass == GetClassByName(getSite()->getOwlModel(), "PolyLine3D"))
	{
		createPolyLine3D(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (isCollectionClass(iInstanceClass))
	{
		OwlInstance* piInstances = nullptr;
		int64_t iInstancesCount = 0;
		GetObjectProperty(
			iInstance,
			GetPropertyByName(getSite()->getOwlModel(), "objects"),
			&piInstances,
			&iInstancesCount);

		for (int64_t iInstanceIndex = 0; iInstanceIndex < iInstancesCount; iInstanceIndex++)
		{
			createGeometry(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
	}
	else if (isReferencePointIndicatorClass(iInstanceClass))
	{
		createReferencePointIndicator(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
	}
	else if (isTransformationClass(iInstanceClass))
	{
		// Reference Point (Anchor) Transformation
		OwlInstance iReferencePointTransformationInstance = iInstance;

		// Reference Point Transformation - matrix
		OwlInstance* piInstances = nullptr;
		int64_t iInstancesCount = 0;
		GetObjectProperty(
			iReferencePointTransformationInstance,
			GetPropertyByName(getSite()->getOwlModel(), "matrix"),
			&piInstances,
			&iInstancesCount);
		assert(iInstancesCount == 1);

		OwlInstance iReferencePointMatrixInstance = piInstances[0];
		assert(iReferencePointMatrixInstance != 0);	

		// Transformation Matrix Transformation
		piInstances = nullptr;
		iInstancesCount = 0;
		GetObjectProperty(
			iReferencePointTransformationInstance,
			GetPropertyByName(getSite()->getOwlModel(), "object"),
			&piInstances,
			&iInstancesCount);
		assert(iInstancesCount == 1);

		OwlInstance iTransformationMatrixTransformationInstance = piInstances[0];
		assert(iTransformationMatrixTransformationInstance != 0);

		if (isTransformationClass(GetInstanceClass(iTransformationMatrixTransformationInstance)))
		{
			// Transformation Matrix Transformation - matrix
			piInstances = nullptr;
			iInstancesCount = 0;
			GetObjectProperty(
				iTransformationMatrixTransformationInstance,
				GetPropertyByName(getSite()->getOwlModel(), "matrix"),
				&piInstances,
				&iInstancesCount);
			assert(iInstancesCount == 1);

			OwlInstance iTransformationMatrixInstance = piInstances[0];
			assert(iTransformationMatrixInstance != 0);

			// Reference Point Transformation - object
			piInstances = nullptr;
			iInstancesCount = 0;
			GetObjectProperty(
				iTransformationMatrixTransformationInstance,
				GetPropertyByName(getSite()->getOwlModel(), "object"),
				&piInstances,
				&iInstancesCount);
			assert(iInstancesCount == 1);

			OwlInstance iRelativeGMLGeometryInstance = piInstances[0];
			assert(iRelativeGMLGeometryInstance != 0);

			iInstanceClass = GetInstanceClass(iRelativeGMLGeometryInstance);
			assert(isCollectionClass(iInstanceClass));

			piInstances = nullptr;
			iInstancesCount = 0;
			GetObjectProperty(
				iRelativeGMLGeometryInstance,
				GetPropertyByName(getSite()->getOwlModel(), "objects"),
				&piInstances,
				&iInstancesCount);
			assert(iInstancesCount == 1);

			OwlInstance iMappedItemGeometryInstance = piInstances[0];
			assert(iMappedItemGeometryInstance != 0);

			auto itMappedItem = m_mapMappedItems.find(iMappedItemGeometryInstance);
			if (itMappedItem == m_mapMappedItems.end())
			{
				vector<SdaiInstance> vecMappedItemGeometryInstances;
				createGeometry(iMappedItemGeometryInstance, vecMappedItemGeometryInstances, false);

				m_mapMappedItems[iMappedItemGeometryInstance] = vecMappedItemGeometryInstances;

				vecGeometryInstances.push_back(
					buildMappedItem(
						vecMappedItemGeometryInstances,
						iReferencePointMatrixInstance,
						iTransformationMatrixInstance)
				);
			}
			else
			{
				vecGeometryInstances.push_back(
					buildMappedItem(
						itMappedItem->second,
						iReferencePointMatrixInstance,
						iTransformationMatrixInstance));
			}
		} // if (isTransformationClass( ...
		else
		{
			wchar_t* szClassName = nullptr;
			GetNameOfClassW(GetInstanceClass(iTransformationMatrixTransformationInstance), &szClassName);

			if (wstring(szClassName) != L"Cube")
			{
				string strEvent = "Internal error; expected 'Cube': '";
				strEvent += CW2A(szClassName);
				strEvent += "'";
				getSite()->logErr(strEvent);
			}
		}
	}
	else
	{
		wchar_t* szClassName = nullptr;
		GetNameOfClassW(iInstanceClass, &szClassName);

		string strEvent = "Geometry is not supported: '";
		strEvent += CW2A(szClassName);
		strEvent += "'";
		getSite()->logErr(strEvent);
	}
}

void _citygml_exporter::createSolid(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	OwlInstance* piInstances = nullptr;
	int64_t iInstancesCount = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "objects"),
		&piInstances,
		&iInstancesCount);

	for (int64_t iInstanceIndex = 0; iInstanceIndex < iInstancesCount; iInstanceIndex++)
	{
		OwlClass iChildInstanceClass = GetInstanceClass(piInstances[iInstanceIndex]);
		assert(iChildInstanceClass != 0);

		if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:SolidType"))
		{
			createSolid(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:CompositeSurfaceType"))
		{
			createCompositeSurface(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:ShellType"))
		{
			createMultiSurface(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else
		{
			//#todo
			wchar_t* szClassName = nullptr;
			GetNameOfClassW(iChildInstanceClass, &szClassName);

			string strEvent = "Geometry is not supported: '";
			strEvent += CW2A(szClassName);
			strEvent += "'";
			getSite()->logErr(strEvent);
		}
	} // for (int64_t iInstanceIndex = ...
}

void _citygml_exporter::createCompositeSolid(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	createMultiSolid(iInstance, vecGeometryInstances, bCreateIfcShapeRepresentation);
}

void _citygml_exporter::createMultiSolid(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	OwlInstance* piInstances = nullptr;
	int64_t iInstancesCount = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "objects"),
		&piInstances,
		&iInstancesCount);

	for (int64_t iInstanceIndex = 0; iInstanceIndex < iInstancesCount; iInstanceIndex++)
	{
		OwlClass iChildInstanceClass = GetInstanceClass(piInstances[iInstanceIndex]);
		assert(iChildInstanceClass != 0);

		if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:SolidType"))
		{
			createSolid(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else
		{
			//#todo
			wchar_t* szClassName = nullptr;
			GetNameOfClassW(iChildInstanceClass, &szClassName);

			string strEvent = "Geometry is not supported: '";
			strEvent += CW2A(szClassName);
			strEvent += "'";
			getSite()->logErr(strEvent);
		}
	} // for (int64_t iInstanceIndex = ...
}

void _citygml_exporter::createMultiSurface(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	OwlInstance* piInstances = nullptr;
	int64_t iInstancesCount = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "objects"),
		&piInstances,
		&iInstancesCount);

	for (int64_t iInstanceIndex = 0; iInstanceIndex < iInstancesCount; iInstanceIndex++)
	{
		OwlClass iChildInstanceClass = GetInstanceClass(piInstances[iInstanceIndex]);
		assert(iChildInstanceClass != 0);

		if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:CompositeSurfaceType"))
		{
			createCompositeSurface(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else if ((iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:SurfacePropertyType")) ||
			(iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:SurfaceType")))
		{
			createSurfaceMember(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "BoundaryRepresentation"))
		{
			createBoundaryRepresentation(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);			
		}
		else 
		{
			//#todo
			wchar_t* szClassName = nullptr;
			GetNameOfClassW(iChildInstanceClass, &szClassName);

			string strEvent = "Geometry is not supported: '";
			strEvent += CW2A(szClassName);
			strEvent += "'";
			getSite()->logErr(strEvent);
		}
	} // for (int64_t iInstanceIndex = ...
}

void _citygml_exporter::createCompositeSurface(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	OwlInstance* piInstances = nullptr;
	int64_t iInstancesCount = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "objects"),
		&piInstances,
		&iInstancesCount);

	for (int64_t iInstanceIndex = 0; iInstanceIndex < iInstancesCount; iInstanceIndex++)
	{
		OwlClass iChildInstanceClass = GetInstanceClass(piInstances[iInstanceIndex]);
		assert(iChildInstanceClass != 0);

		if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:CompositeSurfaceType"))
		{
			createCompositeSurface(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:OrientableSurfaceType"))
		{
			//#todo
			getSite()->logErr("Geometry is not supported: 'class:OrientableSurfaceType'");
		}
		else if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:SurfacePropertyType"))
		{
			createSurfaceMember(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "BoundaryRepresentation"))
		{
			createBoundaryRepresentation(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else
		{
			//#todo
			wchar_t* szClassName = nullptr;
			GetNameOfClassW(iChildInstanceClass, &szClassName);

			string strEvent = "Geometry is not supported: '";
			strEvent += CW2A(szClassName);
			strEvent += "'";
			getSite()->logErr(strEvent);
		}
	} // for (int64_t iInstanceIndex = ...
}

void _citygml_exporter::createSurfaceMember(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	OwlInstance* piInstances = nullptr;
	int64_t iInstancesCount = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "objects"),
		&piInstances,
		&iInstancesCount);

	for (int64_t iInstanceIndex = 0; iInstanceIndex < iInstancesCount; iInstanceIndex++)
	{
		OwlClass iChildInstanceClass = GetInstanceClass(piInstances[iInstanceIndex]);
		assert(iChildInstanceClass != 0);

		if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "class:CompositeSurfaceType"))
		{
			createCompositeSurface(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else if (iChildInstanceClass == GetClassByName(getSite()->getOwlModel(), "BoundaryRepresentation"))
		{
			createBoundaryRepresentation(piInstances[iInstanceIndex], vecGeometryInstances, bCreateIfcShapeRepresentation);
		}
		else
		{
			//#todo
			wchar_t* szClassName = nullptr;
			GetNameOfClassW(iChildInstanceClass, &szClassName);

			string strEvent = "Geometry is not supported: '";
			strEvent += CW2A(szClassName);
			strEvent += "'";
			getSite()->logErr(strEvent);
		}
	} // for (int64_t iInstanceIndex = ...
}

void _citygml_exporter::createBoundaryRepresentation(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);
		
	// Indices
	int64_t* piIndices = nullptr;
	int64_t iIndicesCount = 0;
	GetDatatypeProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "indices"),
		(void**)&piIndices,
		&iIndicesCount);

	// Vertices
	double* pdValue = nullptr;
	int64_t iVerticesCount = 0;
	GetDatatypeProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "vertices"),
		(void**)&pdValue,
		&iVerticesCount);

	vector<SdaiInstance> vecOuterPolygons;
	map<SdaiInstance, vector<SdaiInstance>> mapOuter2InnerPolygons;
	vector<int64_t> vecPolygonIndices;
	map<int64_t, SdaiInstance> mapIndex2Instance;
	for (int64_t iIndex = 0; iIndex < iIndicesCount; iIndex++)
	{
		if (piIndices[iIndex] < 0)
		{
			SdaiInstance iPolyLoopInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyLoop");
			assert(iPolyLoopInstance != 0);

			SdaiAggr pPolygon = sdaiCreateAggrBN(iPolyLoopInstance, "Polygon");
			assert(pPolygon != nullptr);

			for (auto iIndex : vecPolygonIndices)
			{
				assert(mapIndex2Instance.find(iIndex) != mapIndex2Instance.end());

				sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(iIndex));
			}

			if (piIndices[iIndex] == -1)
			{
				// Outer Polygon
				vecOuterPolygons.push_back(iPolyLoopInstance);
			}
			else
			{
				// Outer Polygon : [Inner Polygons...]
				assert(piIndices[iIndex] == -2);
				assert(!vecOuterPolygons.empty());

				auto itOuter2InnerPolygons = mapOuter2InnerPolygons.find(vecOuterPolygons.back());
				if (itOuter2InnerPolygons != mapOuter2InnerPolygons.end())
				{
					itOuter2InnerPolygons->second.push_back(iPolyLoopInstance);
				}
				else
				{
					mapOuter2InnerPolygons[vecOuterPolygons.back()] = vector<SdaiInstance>{ iPolyLoopInstance };
				}
			}

			vecPolygonIndices.clear();

			continue;
		} // if (piIndices[iIndex] < 0)

		vecPolygonIndices.push_back(piIndices[iIndex]);

		if (mapIndex2Instance.find(piIndices[iIndex]) == mapIndex2Instance.end())
		{
			mapIndex2Instance[piIndices[iIndex]] = buildCartesianPointInstance(
				pdValue[(piIndices[iIndex] * 3) + 0],
				pdValue[(piIndices[iIndex] * 3) + 1],
				pdValue[(piIndices[iIndex] * 3) + 2]);
		}
	} // for (int64_t iIndex = ...

	assert(vecPolygonIndices.empty());
	assert(!vecOuterPolygons.empty());

	SdaiInstance iClosedShellInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcClosedShell");
	assert(iClosedShellInstance != 0);

	SdaiAggr pCfsFaces = sdaiCreateAggrBN(iClosedShellInstance, "CfsFaces");
	assert(pCfsFaces != nullptr);	

	for (auto iOuterPolygon : vecOuterPolygons)
	{
		// Outer Polygon
		SdaiInstance iFaceOuterBoundInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcFaceOuterBound");
		assert(iFaceOuterBoundInstance != 0);

		sdaiPutAttrBN(iFaceOuterBoundInstance, "Bound", sdaiINSTANCE, (void*)iOuterPolygon);
		sdaiPutAttrBN(iFaceOuterBoundInstance, "Orientation", sdaiENUM, "T");

		SdaiInstance iFaceInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcFace");
		assert(iFaceInstance != 0);

		SdaiAggr pBounds = sdaiCreateAggrBN(iFaceInstance, "Bounds");
		sdaiAppend(pCfsFaces, sdaiINSTANCE, (void*)iFaceInstance);

		sdaiAppend(pBounds, sdaiINSTANCE, (void*)iFaceOuterBoundInstance);

		// Inner Polygons
		auto itOuter2InnerPolygons = mapOuter2InnerPolygons.find(iOuterPolygon);
		if (itOuter2InnerPolygons != mapOuter2InnerPolygons.end())
		{
			for (auto iInnerPolygon : itOuter2InnerPolygons->second)
			{
				SdaiInstance iFaceBoundInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcFaceBound");
				assert(iFaceBoundInstance != 0);

				sdaiPutAttrBN(iFaceBoundInstance, "Bound", sdaiINSTANCE, (void*)iInnerPolygon);
				sdaiPutAttrBN(iFaceBoundInstance, "Orientation", sdaiENUM, "T");

				sdaiAppend(pBounds, sdaiINSTANCE, (void*)iFaceBoundInstance);
			}
		}
	} // auto iOuterPolygon : ...

	SdaiInstance iFacetedBrepInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcFacetedBrep");
	assert(iFacetedBrepInstance != 0);

	sdaiPutAttrBN(iFacetedBrepInstance, "Outer", sdaiINSTANCE, (void*)iClosedShellInstance);

	createStyledItemInstance(iInstance, iFacetedBrepInstance);

	if (bCreateIfcShapeRepresentation)
	{
		SdaiInstance iShapeRepresentationInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcShapeRepresentation");
		assert(iShapeRepresentationInstance != 0);

		SdaiAggr pItems = sdaiCreateAggrBN(iShapeRepresentationInstance, "Items");
		assert(pItems != 0);

		sdaiAppend(pItems, sdaiINSTANCE, (void*)iFacetedBrepInstance);

		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Brep");

		SdaiInstance iGeometricRepresentationContextInstance = buildGeometricRepresentationContextInstance();
		assert(iGeometricRepresentationContextInstance != 0);

		// CRS #todo
		//const wchar_t* szSrsName = getStringAttributeValue(m_iCurrentOwlBuildingElementInstance, "srsName");
		//if ((szSrsName != nullptr) && (wstring(szSrsName).find(L"EPSG") != string::npos))			
		//{
		//	string strEPSG = getEPSG(szSrsName);
		//	assert(!strEPSG.empty());

		//	SdaiInstance iSourceCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iTargetCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iMapConversion = buildMapConversion(iSourceCRS, iTargetCRS);

		//	double dOrthogonalHeight = 10000; // #todo
		//	sdaiPutAttrBN(iMapConversion, "OrthogonalHeight", sdaiREAL, &dOrthogonalHeight);

		//	double dEastings = 0.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Eastings", sdaiREAL, &dEastings);

		//	double dNorthings = 1.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Northings", sdaiREAL, &dNorthings);			

		//	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "HasCoordinateOperation", sdaiINSTANCE, (void*)iMapConversion);
		//} // if ((szSrsName != nullptr) && ...		

		sdaiPutAttrBN(iShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*)iGeometricRepresentationContextInstance);

		vecGeometryInstances.push_back(iShapeRepresentationInstance);
	}
	else
	{
		vecGeometryInstances.push_back(iFacetedBrepInstance);
	}	
}

void _citygml_exporter::createReferencePointIndicator(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

#pragma region Geometry

	// Matrix
	OwlInstance* piInstances = nullptr;
	int64_t iInstancesCount = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "matrix"),
		&piInstances,
		&iInstancesCount);
	assert(iInstancesCount == 1);

	OwlInstance iMatrixInstance = piInstances[0];
	assert(iMatrixInstance != 0);

	int64_t iValuesCount = 0;
	double* pdValues = nullptr;
	GetDatatypeProperty(
		iMatrixInstance,
		GetPropertyByName(getSite()->getOwlModel(), "coordinates"),
		(void**)&pdValues,
		&iValuesCount);
	assert(iValuesCount == 12);

	// Cube
	piInstances = nullptr;
	iInstancesCount = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "object"),
		&piInstances,
		&iInstancesCount);
	assert(iInstancesCount == 1);

	OwlInstance iCubeInstance = piInstances[0];
	assert(iCubeInstance != 0);

	float fBoundingSphereDiameter = m_fXmax - m_fXmin;
	fBoundingSphereDiameter = max(fBoundingSphereDiameter, m_fYmax - m_fYmin);
	fBoundingSphereDiameter = max(fBoundingSphereDiameter, m_fZmax - m_fZmin);
	float fCubeLength = fBoundingSphereDiameter / 150.f;	

	/*
	Front
	X3 -- X2
	 |    |	
	X0 -- X1

	Back
	X7 -- X6
	 |    |
	X4 -- X5
	*/
	vector<double> vecVertices
	{
		// front
		pdValues[9], pdValues[10], pdValues[11], // X0
		pdValues[9] + fCubeLength, pdValues[10], pdValues[11], // X1
		pdValues[9] + fCubeLength, pdValues[10] + fCubeLength, pdValues[11], // X2
		pdValues[9], pdValues[10] + fCubeLength, pdValues[11], // X3
		// back
		pdValues[9], pdValues[10], pdValues[11] + fCubeLength, // X4
		pdValues[9] + fCubeLength, pdValues[10], pdValues[11] + fCubeLength, // X5
		pdValues[9] + fCubeLength, pdValues[10] + fCubeLength, pdValues[11] + fCubeLength, // X6
		pdValues[9], pdValues[10] + fCubeLength, pdValues[11] + fCubeLength, // X7
	};

	map<int64_t, SdaiInstance> mapIndex2Instance;

	// Front
	// 0
	mapIndex2Instance[0] = buildCartesianPointInstance(
		vecVertices[(0 * 3) + 0],
		vecVertices[(0 * 3) + 1],
		vecVertices[(0 * 3) + 2]);
	// 1
	mapIndex2Instance[1] = buildCartesianPointInstance(
		vecVertices[(1 * 3) + 0],
		vecVertices[(1 * 3) + 1],
		vecVertices[(1 * 3) + 2]);
	// 2
	mapIndex2Instance[2] = buildCartesianPointInstance(
		vecVertices[(2 * 3) + 0],
		vecVertices[(2 * 3) + 1],
		vecVertices[(2 * 3) + 2]);
	// 3
	mapIndex2Instance[3] = buildCartesianPointInstance(
		vecVertices[(3 * 3) + 0],
		vecVertices[(3 * 3) + 1],
		vecVertices[(3 * 3) + 2]);
		
	// Back
	// 4
	mapIndex2Instance[4] = buildCartesianPointInstance(
		vecVertices[(4 * 3) + 0],
		vecVertices[(4 * 3) + 1],
		vecVertices[(4 * 3) + 2]);
	// 5
	mapIndex2Instance[5] = buildCartesianPointInstance(
		vecVertices[(5 * 3) + 0],
		vecVertices[(5 * 3) + 1],
		vecVertices[(5 * 3) + 2]);
	// 6
	mapIndex2Instance[6] = buildCartesianPointInstance(
		vecVertices[(6 * 3) + 0],
		vecVertices[(6 * 3) + 1],
		vecVertices[(6 * 3) + 2]);
	// 7
	mapIndex2Instance[7] = buildCartesianPointInstance(
		vecVertices[(7 * 3) + 0],
		vecVertices[(7 * 3) + 1],
		vecVertices[(7 * 3) + 2]);

#pragma endregion // Geometry

	vector<SdaiInstance> vecPolyLoopInstances;

	// Front
	// 0, 1, 2, 3
	vecPolyLoopInstances.push_back(sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyLoop"));

	SdaiAggr pPolygon = sdaiCreateAggrBN(vecPolyLoopInstances.back(), "Polygon");
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(0));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(1));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(2));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(3));

	// Front
	// 4, 5, 6, 7
	vecPolyLoopInstances.push_back(sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyLoop"));

	pPolygon = sdaiCreateAggrBN(vecPolyLoopInstances.back(), "Polygon");
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(4));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(5));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(6));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(7));

	// Left
	// 4, 0, 3, 7
	vecPolyLoopInstances.push_back(sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyLoop"));

	pPolygon = sdaiCreateAggrBN(vecPolyLoopInstances.back(), "Polygon");
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(4));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(0));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(3));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(7));

	// Right
	// 1, 5, 6, 2
	vecPolyLoopInstances.push_back(sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyLoop"));

	pPolygon = sdaiCreateAggrBN(vecPolyLoopInstances.back(), "Polygon");
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(1));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(5));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(6));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(2));

	// Top
	// 3, 2, 6, 7
	vecPolyLoopInstances.push_back(sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyLoop"));

	pPolygon = sdaiCreateAggrBN(vecPolyLoopInstances.back(), "Polygon");
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(3));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(2));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(6));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(7));

	// Bottom
	// 0, 1, 5, 4
	vecPolyLoopInstances.push_back(sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyLoop"));

	pPolygon = sdaiCreateAggrBN(vecPolyLoopInstances.back(), "Polygon");
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(0));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(1));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(5));
	sdaiAppend(pPolygon, sdaiINSTANCE, (void*)mapIndex2Instance.at(4));

	SdaiInstance iClosedShellInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcClosedShell");
	assert(iClosedShellInstance != 0);

	SdaiAggr pCfsFaces = sdaiCreateAggrBN(iClosedShellInstance, "CfsFaces");
	assert(pCfsFaces != nullptr);

	for (auto iPolyLoopInstance : vecPolyLoopInstances)
	{
		// Outer Polygon
		SdaiInstance iFaceOuterBoundInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcFaceOuterBound");
		assert(iFaceOuterBoundInstance != 0);

		sdaiPutAttrBN(iFaceOuterBoundInstance, "Bound", sdaiINSTANCE, (void*)iPolyLoopInstance);
		sdaiPutAttrBN(iFaceOuterBoundInstance, "Orientation", sdaiENUM, "T");

		SdaiInstance iFaceInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcFace");
		assert(iFaceInstance != 0);

		SdaiAggr pBounds = sdaiCreateAggrBN(iFaceInstance, "Bounds");
		sdaiAppend(pCfsFaces, sdaiINSTANCE, (void*)iFaceInstance);

		sdaiAppend(pBounds, sdaiINSTANCE, (void*)iFaceOuterBoundInstance);
	} // auto iPolyLoopInstance : ...

	SdaiInstance iFacetedBrepInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcFacetedBrep");
	assert(iFacetedBrepInstance != 0);

	sdaiPutAttrBN(iFacetedBrepInstance, "Outer", sdaiINSTANCE, (void*)iClosedShellInstance);

	createStyledItemInstance(iCubeInstance, iFacetedBrepInstance);

	if (bCreateIfcShapeRepresentation)
	{
		SdaiInstance iShapeRepresentationInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcShapeRepresentation");
		assert(iShapeRepresentationInstance != 0);

		SdaiAggr pItems = sdaiCreateAggrBN(iShapeRepresentationInstance, "Items");
		assert(pItems != 0);

		sdaiAppend(pItems, sdaiINSTANCE, (void*)iFacetedBrepInstance);

		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Brep");
		sdaiPutAttrBN(iShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*)buildGeometricRepresentationContextInstance());

		vecGeometryInstances.push_back(iShapeRepresentationInstance);
	}
	else
	{
		vecGeometryInstances.push_back(iFacetedBrepInstance);
	}
}

void _citygml_exporter::createPoint3D(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	int64_t iValuesCount = 0;
	double* pdValue = nullptr;
	GetDatatypeProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "points"),
		(void**)&pdValue,
		&iValuesCount);

	assert(iValuesCount == 3);

	SdaiInstance iCartesianPointInstance = buildCartesianPointInstance(
		pdValue[0],
		pdValue[1],
		pdValue[2]);
	assert(iCartesianPointInstance != 0);

	if (bCreateIfcShapeRepresentation)
	{
		SdaiInstance iShapeRepresentationInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcShapeRepresentation");
		assert(iShapeRepresentationInstance != 0);

		SdaiAggr pItems = sdaiCreateAggrBN(iShapeRepresentationInstance, "Items");
		assert(pItems != 0);

		sdaiAppend(pItems, sdaiINSTANCE, (void*)iCartesianPointInstance);

		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "PointCloud");

		SdaiInstance iGeometricRepresentationContextInstance = buildGeometricRepresentationContextInstance();
		assert(iGeometricRepresentationContextInstance != 0);

		// CRS #todo
		//const wchar_t* szSrsName = getStringAttributeValue(iInstance, "srsName");
		//if ((szSrsName != nullptr) && (wstring(szSrsName).find(L"EPSG") != string::npos))
		//{
		//	string strEPSG = getEPSG(szSrsName);
		//	assert(!strEPSG.empty());

		//	SdaiInstance iSourceCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iTargetCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iMapConversion = buildMapConversion(iSourceCRS, iTargetCRS);

		//	double dOrthogonalHeight = 10000; // #todo
		//	sdaiPutAttrBN(iMapConversion, "OrthogonalHeight", sdaiREAL, &dOrthogonalHeight);

		//	double dEastings = 0.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Eastings", sdaiREAL, &dEastings);

		//	double dNorthings = 1.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Northings", sdaiREAL, &dNorthings);

		//	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "HasCoordinateOperation", sdaiINSTANCE, (void*)iMapConversion);
		//} // if ((szSrsName != nullptr) && ...		

		sdaiPutAttrBN(iShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*)iGeometricRepresentationContextInstance);

		vecGeometryInstances.push_back(iShapeRepresentationInstance);
	}
	else
	{
		vecGeometryInstances.push_back(iCartesianPointInstance);
	}	
}

void _citygml_exporter::createPoint3DSet(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	int64_t iValuesCount = 0;
	double* pdValue = nullptr;
	GetDatatypeProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "points"),
		(void**)&pdValue,
		&iValuesCount);

	assert(iValuesCount >= 3);

	if (bCreateIfcShapeRepresentation)
	{
		SdaiInstance iShapeRepresentationInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcShapeRepresentation");
		assert(iShapeRepresentationInstance != 0);

		SdaiAggr pItems = sdaiCreateAggrBN(iShapeRepresentationInstance, "Items");
		assert(pItems != 0);

		for (int64_t iValue = 0; iValue < iValuesCount; iValue += 3)
		{
			SdaiInstance iCartesianPointInstance = buildCartesianPointInstance(
				pdValue[iValue + 0],
				pdValue[iValue + 1],
				pdValue[iValue + 2]);
			assert(iCartesianPointInstance != 0);

			sdaiAppend(pItems, sdaiINSTANCE, (void*)iCartesianPointInstance);
		} // for (int64_t iValue = ...

		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "PointCloud");

		SdaiInstance iGeometricRepresentationContextInstance = buildGeometricRepresentationContextInstance();
		assert(iGeometricRepresentationContextInstance != 0);

		// CRS #todo
		//const wchar_t* szSrsName = getStringAttributeValue(iInstance, "srsName");
		//if ((szSrsName != nullptr) && (wstring(szSrsName).find(L"EPSG") != string::npos))
		//{
		//	string strEPSG = getEPSG(szSrsName);
		//	assert(!strEPSG.empty());

		//	SdaiInstance iSourceCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iTargetCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iMapConversion = buildMapConversion(iSourceCRS, iTargetCRS);

		//	double dOrthogonalHeight = 10000; // #todo
		//	sdaiPutAttrBN(iMapConversion, "OrthogonalHeight", sdaiREAL, &dOrthogonalHeight);

		//	double dEastings = 0.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Eastings", sdaiREAL, &dEastings);

		//	double dNorthings = 1.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Northings", sdaiREAL, &dNorthings);

		//	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "HasCoordinateOperation", sdaiINSTANCE, (void*)iMapConversion);
		//} // if ((szSrsName != nullptr) && ...		

		sdaiPutAttrBN(iShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*)iGeometricRepresentationContextInstance);

		vecGeometryInstances.push_back(iShapeRepresentationInstance);
	}
	else
	{
		for (int64_t iValue = 0; iValue < iValuesCount; iValue += 3)
		{
			SdaiInstance iCartesianPointInstance = buildCartesianPointInstance(
				pdValue[iValue + 0],
				pdValue[iValue + 1],
				pdValue[iValue + 2]);
			assert(iCartesianPointInstance != 0);

			vecGeometryInstances.push_back(iCartesianPointInstance);
		} // for (int64_t iValue = ...
	}	
}

void _citygml_exporter::createPolyLine3D(OwlInstance iInstance, vector<SdaiInstance>& vecGeometryInstances, bool bCreateIfcShapeRepresentation)
{
	assert(iInstance != 0);

	int64_t iValuesCount = 0;
	double* pdValue = nullptr;
	GetDatatypeProperty(
		iInstance,
		GetPropertyByName(getSite()->getOwlModel(), "points"),
		(void**)&pdValue,
		&iValuesCount);

	assert(iValuesCount >= 6);

	SdaiInstance iPolyLineInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcPolyline");
	assert(iPolyLineInstance != 0);

	SdaiAggr pPoints = sdaiCreateAggrBN(iPolyLineInstance, "Points");
	assert(pPoints != nullptr);

	for (int64_t iValue = 0; iValue < iValuesCount; iValue += 3)
	{
		SdaiInstance iCartesianPointInstance = buildCartesianPointInstance(
			pdValue[iValue + 0],
			pdValue[iValue + 1],
			pdValue[iValue + 2]);
		assert(iCartesianPointInstance != 0);

		sdaiAppend(pPoints, sdaiINSTANCE, (void*)iCartesianPointInstance);
	} // for (int64_t iValue = ...

	if (bCreateIfcShapeRepresentation)
	{
		SdaiInstance iShapeRepresentationInstance = sdaiCreateInstanceBN(getSdaiModel(), "IfcShapeRepresentation");
		assert(iShapeRepresentationInstance != 0);

		SdaiAggr pItems = sdaiCreateAggrBN(iShapeRepresentationInstance, "Items");
		assert(pItems != 0);

		sdaiAppend(pItems, sdaiINSTANCE, (void*)iPolyLineInstance);

		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
		sdaiPutAttrBN(iShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Curve3D");

		SdaiInstance iGeometricRepresentationContextInstance = buildGeometricRepresentationContextInstance();
		assert(iGeometricRepresentationContextInstance != 0);

		// CRS #todo
		//const wchar_t* szSrsName = getStringAttributeValue(iInstance, "srsName");
		//if ((szSrsName != nullptr) && (wstring(szSrsName).find(L"EPSG") != string::npos))
		//{
		//	string strEPSG = getEPSG(szSrsName);
		//	assert(!strEPSG.empty());

		//	SdaiInstance iSourceCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iTargetCRS = buildProjectedCRS(strEPSG);
		//	SdaiInstance iMapConversion = buildMapConversion(iSourceCRS, iTargetCRS);

		//	double dOrthogonalHeight = 10000; // #todo
		//	sdaiPutAttrBN(iMapConversion, "OrthogonalHeight", sdaiREAL, &dOrthogonalHeight);

		//	double dEastings = 0.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Eastings", sdaiREAL, &dEastings);

		//	double dNorthings = 1.; // #todo
		//	sdaiPutAttrBN(iMapConversion, "Northings", sdaiREAL, &dNorthings);

		//	sdaiPutAttrBN(iGeometricRepresentationContextInstance, "HasCoordinateOperation", sdaiINSTANCE, (void*)iMapConversion);
		//} // if ((szSrsName != nullptr) && ...		

		sdaiPutAttrBN(iShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*)iGeometricRepresentationContextInstance);

		vecGeometryInstances.push_back(iShapeRepresentationInstance);
	}
	else
	{
		vecGeometryInstances.push_back(iPolyLineInstance);
	}	
}

void _citygml_exporter::createProperties(OwlInstance iOwlInstance, SdaiInstance iSdaiInstance)
{
	assert(iOwlInstance != 0);
	assert(iSdaiInstance != 0);

	map<string, SdaiInstance> mapProperties;

	RdfProperty iPropertyInstance = GetInstancePropertyByIterator(iOwlInstance, 0);
	while (iPropertyInstance != 0)
	{
		char* szPropertyName = nullptr;
		GetNameOfProperty(iPropertyInstance, &szPropertyName);

		string strPropertyName = szPropertyName;
		
		if (strPropertyName.find("prop:") == 0)
		{
			// Properties
			switch (GetPropertyType(iPropertyInstance))
			{
				case DATATYPEPROPERTY_TYPE_DOUBLE:
				{
					double* pdValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iOwlInstance, iPropertyInstance, (void**)&pdValue, &iValuesCount);

					assert(iValuesCount == 1);

					mapProperties[szPropertyName] = buildPropertySingleValueReal(
						strPropertyName.c_str(),
						"attribute",
						pdValue[0],
						"IFCREAL");
				}
				break;

				case DATATYPEPROPERTY_TYPE_WCHAR_T_ARRAY:
				{
					wchar_t** szValue = nullptr;
					int64_t iValuesCount = 0;
					GetDatatypeProperty(iOwlInstance, iPropertyInstance, (void**)&szValue, &iValuesCount);

					mapProperties[szPropertyName] = buildPropertySingleValueText(
						strPropertyName.c_str(),
						"attribute",
						(LPCSTR)CW2A(szValue[0]),
						"IFCTEXT");
				}
				break;

				case OBJECTTYPEPROPERTY_TYPE:
				{
					//#todo
				}
				break;

				default:
				{
					//#todo
				}
				break;
			} // switch (GetPropertyType(iPropertyInstance))
		} // prop:
		else if (strPropertyName.find("attr:") == 0)
		{
			// Attributes
			assert(GetPropertyType(iPropertyInstance) == DATATYPEPROPERTY_TYPE_WCHAR_T_ARRAY);

			wchar_t** szValue = nullptr;
			int64_t iValuesCount = 0;
			GetDatatypeProperty(iOwlInstance, iPropertyInstance, (void**)&szValue, &iValuesCount);

			assert(iValuesCount == 1);

			mapProperties[szPropertyName] = buildPropertySingleValueText(
				strPropertyName.c_str(),
				"attribute",
				(LPCSTR)CW2A(szValue[0]),
				"IFCTEXT");
		} // attr:

		iPropertyInstance = GetInstancePropertyByIterator(iOwlInstance, iPropertyInstance);
	} // while (iPropertyInstance != 0)

	if (mapProperties.empty())
	{
		return;
	}

	SdaiAggr pHasProperties = nullptr;
	SdaiInstance iPropertySetInstance = buildPropertySet("Attributes & Properties", pHasProperties);

	for (auto itProperty : mapProperties)
	{
		sdaiAppend(pHasProperties, sdaiINSTANCE, (void*)itProperty.second);
	}

	buildRelDefinesByProperties(iSdaiInstance, iPropertySetInstance);
}

SdaiInstance _citygml_exporter::buildBuildingElementInstance(
	OwlInstance iOwlInstance,
	_matrix* pMatrix,
	SdaiInstance iPlacementRelativeTo,
	SdaiInstance& iBuildingElementInstancePlacement,
	const vector<SdaiInstance>& vecRepresentations)
{
	assert(iOwlInstance != 0);
	assert(pMatrix != nullptr);
	assert(iPlacementRelativeTo != 0);
	assert(!vecRepresentations.empty());

	string strTag = getTag(iOwlInstance);

	OwlClass iInstanceClass = GetInstanceClass(iOwlInstance);
	assert(iInstanceClass != 0);

	char* szClassName = nullptr;
	GetNameOfClass(iInstanceClass, &szClassName);
	assert(szClassName != nullptr);

	string strClass = szClassName;

	string strEntity = "IfcBuildingElement";
	if (isWallSurfaceClass(iInstanceClass))
	{
		strEntity = "IfcWall";
	}
	else if (isRoofSurfaceClass(iInstanceClass))
	{
		strEntity = "IfcRoof";
	}
	else if (isDoorClass(iInstanceClass))
	{
		strEntity = "IfcDoor";
	}
	else if (isWindowClass(iInstanceClass))
	{
		strEntity = "IfcWindow";
	}
	else
	{
		strEntity = "IfcBuildingElementProxy"; // Proxy/Unknown Building Element
	}

	return _exporter_base::buildBuildingElementInstance(
		strEntity.c_str(),
		strTag.c_str(),
		szClassName,
		pMatrix,
		iPlacementRelativeTo,
		iBuildingElementInstancePlacement,
		vecRepresentations);
}

bool _citygml_exporter::isCollectionClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iCollectionClass) || IsClassAncestor(iInstanceClass, m_iCollectionClass);
}

bool _citygml_exporter::isTransformationClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iTransformationClass) || IsClassAncestor(iInstanceClass, m_iTransformationClass);
}

void _citygml_exporter::getInstancesDefaultState()
{
	m_mapInstanceDefaultState.clear();

	// Enable only unreferenced instances
	OwlInstance iInstance = GetInstancesByIterator(getSite()->getOwlModel(), 0);
	while (iInstance != 0)
	{
		m_mapInstanceDefaultState[iInstance] = GetInstanceInverseReferencesByIterator(iInstance, 0) == 0;

		iInstance = GetInstancesByIterator(getSite()->getOwlModel(), iInstance);
	}

	// Enable children/descendants with geometry
	for (auto& itInstanceDefaultState : m_mapInstanceDefaultState)
	{
		if (!itInstanceDefaultState.second)
		{
			continue;
		}

		if (!GetInstanceGeometryClass(itInstanceDefaultState.first) ||
			!GetBoundingBox(itInstanceDefaultState.first, nullptr, nullptr))
		{
			OwlClass iNillClass = GetClassByName(getSite()->getOwlModel(), "Nill");

			OwlClass iInstanceClass = GetInstanceClass(itInstanceDefaultState.first);
			assert(iInstanceClass != 0);

			if ((iInstanceClass != iNillClass) && !IsClassAncestor(iInstanceClass, iNillClass))
			{
				getInstanceDefaultStateRecursive(itInstanceDefaultState.first);
			}
		}
	}
}

void _citygml_exporter::getInstanceDefaultStateRecursive(OwlInstance iInstance)
{
	assert(iInstance != 0);

	RdfProperty iProperty = GetInstancePropertyByIterator(iInstance, 0);
	while (iProperty != 0)
	{
		if (GetPropertyType(iProperty) == OBJECTPROPERTY_TYPE)
		{
			int64_t iValuesCount = 0;
			OwlInstance* piValues = nullptr;
			GetObjectProperty(iInstance, iProperty, &piValues, &iValuesCount);

			for (int64_t iValue = 0; iValue < iValuesCount; iValue++)
			{
				if ((piValues[iValue] != 0) &&
					!m_mapInstanceDefaultState.at(piValues[iValue]))
				{
					// Enable to avoid infinity recursion
					m_mapInstanceDefaultState.at(piValues[iValue]) = true;

					if (!GetInstanceGeometryClass(piValues[iValue]) ||
						!GetBoundingBox(piValues[iValue], nullptr, nullptr))
					{
						getInstanceDefaultStateRecursive(piValues[iValue]);
					}
				}
			}
		}

		iProperty = GetInstancePropertyByIterator(iInstance, iProperty);
	}
}

OwlClass _citygml_exporter::isCityModelClass(OwlClass iInstanceClass) const
{
	return (iInstanceClass == m_iCityModelClass) || IsClassAncestor(iInstanceClass, m_iCityModelClass);
}

OwlClass _citygml_exporter::isBoundingShapeClass(OwlClass iInstanceClass) const
{
	return (iInstanceClass == m_iBoundingShapeClass) || IsClassAncestor(iInstanceClass, m_iBoundingShapeClass);
}

OwlClass _citygml_exporter::isEnvelopeClass(OwlClass iInstanceClass) const
{
	return (iInstanceClass == m_iEnvelopeClass) || IsClassAncestor(iInstanceClass, m_iEnvelopeClass);
}

bool _citygml_exporter::isBuildingElement(OwlInstance iInstance) const
{
	assert(iInstance != 0);

	OwlClass iInstanceClass = GetInstanceClass(iInstance);
	assert(iInstanceClass != 0);

	if (isWallSurfaceClass(iInstanceClass))
	{
		return true;
	}

	if (isRoofSurfaceClass(iInstanceClass))
	{
		return true;
	}
	
	if (isDoorClass(iInstanceClass))
	{
		return true;
	}
	
	if (isWindowClass(iInstanceClass))
	{
		return true;
	}

	return false;
}

bool _citygml_exporter::isBuildingClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iBuildingClass) || IsClassAncestor(iInstanceClass, m_iBuildingClass);
}

bool _citygml_exporter::isWallSurfaceClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iWallSurfaceClass) || IsClassAncestor(iInstanceClass, m_iWallSurfaceClass);
}

bool _citygml_exporter::isRoofSurfaceClass(OwlInstance iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iRoofSurfaceClass) || IsClassAncestor(iInstanceClass, m_iRoofSurfaceClass);
}

bool _citygml_exporter::isDoorClass(OwlInstance iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iDoorClass) || IsClassAncestor(iInstanceClass, m_iDoorClass);
}

bool _citygml_exporter::isWindowClass(OwlInstance iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iWindowClass) || IsClassAncestor(iInstanceClass, m_iWindowClass);
}

bool _citygml_exporter::isCadastralParcelClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iCadastralParcelClass) || IsClassAncestor(iInstanceClass, m_iCadastralParcelClass);
}

bool _citygml_exporter::isReferencePointIndicatorClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iReferencePointIndicatorClass) || IsClassAncestor(iInstanceClass, m_iReferencePointIndicatorClass);
}

bool _citygml_exporter::isFeatureClass(OwlInstance iInstanceClass) const
{
	assert(iInstanceClass != 0);

	if (isVegetationObjectClass(iInstanceClass) ||
		isWaterObjectClass(iInstanceClass) ||
		isBridgeObjectClass(iInstanceClass) ||
		isTunnelObjectClass(iInstanceClass) ||
		isTransportationObjectClass(iInstanceClass) ||
		isTrafficSpaceClass(iInstanceClass) ||
		isTrafficAreaClass(iInstanceClass) ||
		isFurnitureObjectClass(iInstanceClass) ||
		isReliefObjectClass(iInstanceClass) ||
		isLandUseClass(iInstanceClass) ||
		isCadastralParcelClass(iInstanceClass))
	{
		return true;
	}

	return false;
}

bool _citygml_exporter::isVegetationObjectClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iVegetationObjectClass) || IsClassAncestor(iInstanceClass, m_iVegetationObjectClass);
}

bool _citygml_exporter::isWaterObjectClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iWaterObjectClass) || IsClassAncestor(iInstanceClass, m_iWaterObjectClass);
}

bool _citygml_exporter::isBridgeObjectClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iBridgeObjectClass) || IsClassAncestor(iInstanceClass, m_iBridgeObjectClass);
}

bool _citygml_exporter::isTunnelObjectClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iTunnelObjectClass) || IsClassAncestor(iInstanceClass, m_iTunnelObjectClass);
}

bool _citygml_exporter::isTransportationObjectClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iTransportationObjectClass) || IsClassAncestor(iInstanceClass, m_iTransportationObjectClass);
}

bool _citygml_exporter::isFurnitureObjectClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iFurnitureObjectClass) || IsClassAncestor(iInstanceClass, m_iFurnitureObjectClass);
}

bool _citygml_exporter::isReliefObjectClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iReliefObjectClass) || IsClassAncestor(iInstanceClass, m_iReliefObjectClass);
}

bool _citygml_exporter::isLandUseClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iLandUseClass) || IsClassAncestor(iInstanceClass, m_iLandUseClass);
}

bool _citygml_exporter::isTrafficSpaceClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iTrafficSpaceClass) || IsClassAncestor(iInstanceClass, m_iTrafficSpaceClass);
}

bool _citygml_exporter::isTrafficAreaClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iTrafficAreaClass) || IsClassAncestor(iInstanceClass, m_iTrafficAreaClass);
}

bool _citygml_exporter::isUnknownClass(OwlClass iInstanceClass) const
{
	assert(iInstanceClass != 0);

	return (iInstanceClass == m_iThingClass) || IsClassAncestor(iInstanceClass, m_iThingClass);
}

void _citygml_exporter::collectSRSData(OwlInstance iRootInstance)
{
	OwlInstance iInstance = GetInstancesByIterator(getSite()->getOwlModel(), 0);
	while (iInstance != 0)
	{
		OwlClass iInstanceClass = GetInstanceClass(iInstance);
		assert(iInstanceClass != 0);

		if (isEnvelopeClass(iInstanceClass))
		{
			OwlInstance iEnvelopeInstance = iInstance;

			OwlInstance iParentInstance = GetInstanceInverseReferencesByIterator(iInstance, 0);
			if (iParentInstance != 0)
			{
				OwlClass iParentInstanceClass = GetInstanceClass(iParentInstance);
				assert(iParentInstanceClass != 0);

				if (isBoundingShapeClass(iParentInstanceClass))
				{
					iParentInstance = GetInstanceInverseReferencesByIterator(iParentInstance, 0);
					if (iParentInstance != 0)
					{
						iParentInstanceClass = GetInstanceClass(iParentInstance);
						assert(iParentInstanceClass != 0);

						if (isCityModelClass(iParentInstanceClass))
						{
							m_iModelEnvelopeInstance = iEnvelopeInstance;
						}
						else if (isBuildingClass(iParentInstanceClass))
						{
							assert(m_mapBuildingEnvelopeInstance.find(iInstance) == m_mapBuildingEnvelopeInstance.end());
							m_mapBuildingEnvelopeInstance[iInstance] = iEnvelopeInstance;
						}
					} // if (iParentInstance != 0)	
				} // if (isBoundingShapeClass(iParentInstanceClass))
			} // if (iParentInstance != 0)
		} // if (isEnvelopeClass(iInstanceClass))

		iInstance = GetInstancesByIterator(getSite()->getOwlModel(), iInstance);
	} // while (iInstance != 0)
}

bool _citygml_exporter::retrieveEnvelopeSRSData(OwlInstance iEnvelopeInstance, string& strEPSGCode, vector<double>& vecLowerCorner, vector<double>& vecUpperCorner)
{
	assert(iEnvelopeInstance != 0);

	strEPSGCode = "";
	vecLowerCorner.clear();
	vecUpperCorner.clear();

	string strSrsName = getStringAttributeValue(iEnvelopeInstance, "srsName");
	if (!strSrsName.empty() && (strSrsName.find("EPSG") != string::npos))
	{
		strEPSGCode = getEPSGCode(strSrsName);
		assert(!strEPSGCode.empty());

		OwlInstance* piInstances = nullptr;
		int64_t iInstancesCount = 0;
		GetObjectProperty(
			iEnvelopeInstance,
			GetPropertyByName(getSite()->getOwlModel(), "$relations"),
			&piInstances,
			&iInstancesCount);
		assert(iInstancesCount == 2);

		OwlInstance iLowerCornerInstance = 0;
		OwlInstance iUpperCornerInstance = 0;

		string strTag0 = getTag(piInstances[0]);
		string strTag1 = getTag(piInstances[1]);

		if (_string::endsWith(strTag0, "lowerCorner"))
		{
			iLowerCornerInstance = piInstances[0];

			if (_string::endsWith(strTag1, "upperCorner"))
			{
				iUpperCornerInstance = piInstances[1];
			}
			else
			{
				assert(false);

				return false;
			}
		}
		else if (_string::endsWith(strTag1, "lowerCorner"))
		{
			iLowerCornerInstance = piInstances[1];

			if (_string::endsWith(strTag0, "upperCorner"))
			{
				iUpperCornerInstance = piInstances[0];
			}
			else
			{
				assert(false);

				return false;
			}
		}
		else
		{
			assert(false);

			return false;
		}

		// lowerCorner
		wchar_t** szValue = nullptr;
		int64_t iValuesCount = 0;
		GetDatatypeProperty(
			iLowerCornerInstance,
			GetPropertyByName(getSite()->getOwlModel(), "value"),
			(void**)&szValue,
			&iValuesCount);
		assert(iValuesCount == 1);

		getPosValuesW(szValue[0], vecLowerCorner);

		// upperCorner
		szValue = nullptr;
		iValuesCount = 0;
		GetDatatypeProperty(
			iUpperCornerInstance,
			GetPropertyByName(getSite()->getOwlModel(), "value"),
			(void**)&szValue,
			&iValuesCount);
		assert(iValuesCount == 1);

		getPosValuesW(szValue[0], vecUpperCorner);

		return true;
	} // if (!strSrsName.empty() && ...

	return false;
}

bool _citygml_exporter::retrieveEnvelopeSRSData(OwlInstance iEnvelopeInstance, string& strEPSGCode, vector<double>& vecCentroid)
{
	assert(iEnvelopeInstance != 0);

	strEPSGCode = "";
	vecCentroid.clear();

	vector<double> vecLowerCorner;
	vector<double> vecUpperCorner;
	if (retrieveEnvelopeSRSData(iEnvelopeInstance, strEPSGCode, vecLowerCorner, vecUpperCorner))
	{
		vecCentroid.push_back((vecLowerCorner[0] + vecUpperCorner[0]) / 2.);
		vecCentroid.push_back((vecLowerCorner[1] + vecUpperCorner[1]) / 2.);
		vecCentroid.push_back((vecLowerCorner[2] + vecUpperCorner[2]) / 2.);

		return true;
	}

	return false;
}

bool _citygml_exporter::transformEnvelopeSRSDataAsync(OwlInstance iEnvelopeInstance)
{
	assert(iEnvelopeInstance != 0);

	string strEPSGCode;
	vector<double> vecCentroid;
	if (retrieveEnvelopeSRSData(iEnvelopeInstance, strEPSGCode, vecCentroid))
	{
		return getSite()->toWGS84Async(
			atoi(strEPSGCode.c_str()),
			(float)vecCentroid[0],
			(float)vecCentroid[1],
			(float)vecCentroid[2]);
	}

	return false;
}

// ************************************************************************************************
_gml_exporter::_gml_exporter(_gml2ifc_exporter* pSite)
	: _citygml_exporter(pSite)
{}

/*virtual*/ _gml_exporter::~_gml_exporter()
{}

// ************************************************************************************************
_cityjson_exporter::_cityjson_exporter(_gml2ifc_exporter* pSite)
	: _citygml_exporter(pSite)
{}

/*virtual*/ _cityjson_exporter::~_cityjson_exporter()
{}
