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
#include "BRepIfc.h"

extern  int_t     model;

extern  int_t     * aggrRelatedElements,
                * aggrRepresentations;


void    createIfcBRepShape(shellStruct * pShell)
{
	ASSERT(aggrRepresentations);

    sdaiAppend(aggrRepresentations, sdaiINSTANCE, (void*) buildShapeRepresentationInstance(pShell));
}


//
//
//		ShapeRepresentation
//
//


int_t		buildShapeRepresentationInstance(shellStruct * pShell)
{
	int_t		ifcShapeRepresentationInstance, * aggrItems;

	ifcShapeRepresentationInstance = sdaiCreateInstanceBN(model, "IFCSHAPEREPRESENTATION");

	aggrItems = sdaiCreateAggrBN(ifcShapeRepresentationInstance, "Items");

	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationIdentifier", sdaiSTRING, "Body");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "RepresentationType", sdaiSTRING, "Brep");
	sdaiPutAttrBN(ifcShapeRepresentationInstance, "ContextOfItems", sdaiINSTANCE, (void*) getGeometricRepresentationContextInstance());

    while  (pShell) {
        POLYGON3DSTRUCT   * pPolygon = pShell->pPolygon;
        int_t     ifcFacetedBrepInstance, ifcClosedShellInstance, * aggrCfsFaces;

	    ifcClosedShellInstance = sdaiCreateInstanceBN(model, "IFCCLOSEDSHELL");
	    aggrCfsFaces = sdaiCreateAggrBN(ifcClosedShellInstance, "CfsFaces");

        while  (pPolygon) {
            VECTOR3DSTRUCT  * pVector = pPolygon->pVector;
	        int_t		ifcPolyLoopInstance, * aggrPolygon,
                    ifcFaceOuterBoundInstance,
                    ifcFaceInstance, * aggrBounds;

	        ifcPolyLoopInstance = sdaiCreateInstanceBN(model, "IFCPOLYLOOP");
	        aggrPolygon = sdaiCreateAggrBN(ifcPolyLoopInstance, "Polygon");

            while  (pVector) {
                POINT3DSTRUCT   * pPoint = pVector->pPoint;
                //
                //  Check if point is already written
                //
                if  (! pPoint->ifcCartesianPointInstance) {
                    pPoint->ifcCartesianPointInstance = buildCartesianPointInstance(pPoint->x, pPoint->y, pPoint->z);
                }
                            
                sdaiAppend(aggrPolygon, sdaiINSTANCE, (void *) pPoint->ifcCartesianPointInstance);

                pVector = pVector->next;
            }

	        ifcFaceOuterBoundInstance = sdaiCreateInstanceBN(model, "IFCFACEOUTERBOUND");
	        sdaiPutAttrBN(ifcFaceOuterBoundInstance, "Bound", sdaiINSTANCE, (void *) ifcPolyLoopInstance);
            sdaiPutAttrBN(ifcFaceOuterBoundInstance, "Orientation", sdaiENUM, "T");

	        ifcFaceInstance = sdaiCreateInstanceBN(model, "IFCFACE");
	        aggrBounds = sdaiCreateAggrBN(ifcFaceInstance, "Bounds");
	        sdaiAppend(aggrBounds, sdaiINSTANCE, (void *) ifcFaceOuterBoundInstance);

	        sdaiAppend(aggrCfsFaces, sdaiINSTANCE, (void *) ifcFaceInstance);

            if  (pPolygon->pOpeningVector) {
	            int_t		ifcFaceBoundInstance;

                pVector = pPolygon->pOpeningVector;

	            ifcPolyLoopInstance = sdaiCreateInstanceBN(model, "IFCPOLYLOOP");
	            aggrPolygon = sdaiCreateAggrBN(ifcPolyLoopInstance, "Polygon");

                while  (pVector) {
                    POINT3DSTRUCT   * pPoint = pVector->pPoint;
                    //
                    //  Check if point is already written
                    //
                    if  (! pPoint->ifcCartesianPointInstance) {
                        pPoint->ifcCartesianPointInstance = buildCartesianPointInstance(pPoint->x, pPoint->y, pPoint->z);
                    }
                            
                    sdaiAppend(aggrPolygon, sdaiINSTANCE, (void *) pPoint->ifcCartesianPointInstance);

                    pVector = pVector->next;
                }

	            ifcFaceBoundInstance = sdaiCreateInstanceBN(model, "IFCFACEBOUND");
	            sdaiPutAttrBN(ifcFaceBoundInstance, "Bound", sdaiINSTANCE, (void *) ifcPolyLoopInstance);
                sdaiPutAttrBN(ifcFaceBoundInstance, "Orientation", sdaiENUM, "T");

	            sdaiAppend(aggrBounds, sdaiINSTANCE, (void *) ifcFaceBoundInstance);
            }

            pPolygon = pPolygon->next;
        }

	    ifcFacetedBrepInstance = sdaiCreateInstanceBN(model, "IFCFACETEDBREP");
	    sdaiPutAttrBN(ifcFacetedBrepInstance, "Outer", sdaiINSTANCE, (void *) ifcClosedShellInstance);
	
        sdaiAppend(aggrItems, sdaiINSTANCE, (void*) ifcFacetedBrepInstance);

        pShell = pShell->next;
    }

	ASSERT(ifcShapeRepresentationInstance);

	return	ifcShapeRepresentationInstance;
}


