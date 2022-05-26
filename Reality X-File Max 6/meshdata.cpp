//-----------------------------------------------------------------------------
// File: MeshData.cpp
//
// Desc: Functions used to massage mesh data into a format useable in X Files
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "pch.h"
#include "MeshData.h"

BOOL FindIdenticalVertex
    (
    SMeshData *pMeshData, 
    DWORD iVertexIndex, 
    DWORD iSmoothingGroup, 
    DWORD iTextureIndex,
    DWORD iMaterial,
    DWORD *piVertex
    )
{
    DWORD iHeadVertex;
    DWORD iCurVertex;

    // walk the wedge list to see if any of the other vertice have the same smoothing and texture index
    iHeadVertex = iVertexIndex;
    iCurVertex = iHeadVertex;
    do
    {
        // if identical requirements, then return true with the vertex index
        if ((pMeshData->m_rgVertices[iCurVertex].iSmoothingGroupIndex == iSmoothingGroup)
            && (pMeshData->m_rgVertices[iCurVertex].iTextureIndex == iTextureIndex) 
            && (pMeshData->m_rgVertices[iCurVertex].iMaterial == iMaterial) )
        {
            *piVertex = iCurVertex;
            return TRUE;
        }

        // move to next element of wedge list and check to see if we wrapped (circular list)
        iCurVertex = pMeshData->m_rgVertices[iCurVertex].iWedgeList;
    }
    while (iHeadVertex != iCurVertex);

    return FALSE;
}

void FindNormal
    (
    SMeshData *pMeshData,
    DWORD iVertexIndex, 
    DWORD iSmoothingGroup,
    Point3 &vNormalNew 
    )
{
    DWORD iHeadVertex;
    DWORD iCurVertex;

    // walk the wedge list to find other split vertices with the same smoothing group
    iHeadVertex = iVertexIndex;
    iCurVertex = iHeadVertex;
    do
    {
        // if same smoothing group add the normal in
        if (pMeshData->m_rgVertices[iCurVertex].iSmoothingGroupIndex == iSmoothingGroup)    
        {
            vNormalNew = pMeshData->m_rgVertices[iCurVertex].vNormal;
            return;
        }

        // move to next element of wedge list and check to see if we wrapped (circular list)
        iCurVertex = pMeshData->m_rgVertices[iCurVertex].iWedgeList;
    }
    while (iHeadVertex != iCurVertex);

    //.first vertex in the smoothing group, set normal to zero
    vNormalNew.x = 0.0f;
    vNormalNew.y = 0.0f;
    vNormalNew.z = 0.0f;
}

void AddNormalContribution
    (
    SMeshData *pMeshData,
    DWORD iVertexIndex, 
    DWORD iSmoothingGroup,
    Point3 vNormal 
    )
{
    DWORD iHeadVertex;
    DWORD iCurVertex;

    // walk the wedge list to find other split vertices with the same smoothing group
    iHeadVertex = iVertexIndex;
    iCurVertex = iHeadVertex;
    do
    {
        // if same smoothing group add the normal in
        if (pMeshData->m_rgVertices[iCurVertex].iSmoothingGroupIndex == iSmoothingGroup)    
        {
            pMeshData->m_rgVertices[iCurVertex].vNormal += vNormal;
        }

        // move to next element of wedge list and check to see if we wrapped (circular list)
        iCurVertex = pMeshData->m_rgVertices[iCurVertex].iWedgeList;
    }
    while (iHeadVertex != iCurVertex);

}

HRESULT GenerateMeshData
    (
    Mesh *pMesh,
	INode *pNode,
    SMeshData *pMeshData,
    DWORD *rgdwMeshMaterials
    )
{
    HRESULT hr = S_OK;
    BOOL *rgbVertexReferencedArray = NULL;
    DWORD cVerticesMax;
    DWORD iRawVertexIndex;
    DWORD iTextureIndex;
    DWORD iSmoothingGroupIndex;
    DWORD iVertex;
    DWORD iFace;
    DWORD iPoint;
    DWORD iNewVertex;
    BOOL bFound;
    SVertexData *rgVerticesNew;
    DWORD iMaterial;

    assert(pMesh != NULL);
    assert(pMeshData != NULL);

    pMeshData->m_bTexCoordsPresent = FALSE;
    pMeshData->m_cFaces = pMesh->numFaces;
    pMeshData->m_cVertices = pMesh->numVerts;
    pMeshData->m_cVerticesBeforeDuplication = pMesh->numVerts;
    cVerticesMax = pMesh->numVerts;

    pMeshData->m_rgVertices = new SVertexData[cVerticesMax];
	memset(pMeshData->m_rgVertices, 0, sizeof(SVertexData)*cVerticesMax);
    pMeshData->m_rgFaces = new SFaceData[pMeshData->m_cFaces];
	memset(pMeshData->m_rgFaces, 0, sizeof(SFaceData)*pMeshData->m_cFaces);
    rgbVertexReferencedArray = new BOOL[pMeshData->m_cVerticesBeforeDuplication];
	memset(rgbVertexReferencedArray, 0, sizeof(BOOL)*pMeshData->m_cVerticesBeforeDuplication);

    if ((pMeshData->m_rgVertices == NULL) 
            || (pMeshData->m_rgFaces == NULL)
            || (rgbVertexReferencedArray == NULL))
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }


    if( !(pMesh->normalsBuilt) )
    {
        pMesh->checkNormals(TRUE);
    }

    // Initialize vertex node list so that first batch are the same as the
    // vertex array in the mesh.  Duplicated vertices will be appended to the list.
    // The first time a vertex comes up in face enumeration below, the initial vertex
    // node added here will be modified to reflect the smoothing and texture info.
    // When the vertex comes up again with different smoothing or texture info, the
    // vertex will be duplicated and appended to the list.
    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++ )
    {
        rgbVertexReferencedArray[iVertex] = FALSE;

        pMeshData->m_rgVertices[iVertex].iPointRep = iVertex;
        pMeshData->m_rgVertices[iVertex].iWedgeList = iVertex;

        // default values that should be reset if the vertex is actually used by a face...
        pMeshData->m_rgVertices[iVertex].vNormal.x = 0;
        pMeshData->m_rgVertices[iVertex].vNormal.y = 0;
        pMeshData->m_rgVertices[iVertex].vNormal.z = 0;
        pMeshData->m_rgVertices[iVertex].iSmoothingGroupIndex = 0;
        pMeshData->m_rgVertices[iVertex].iTextureIndex = 0;
        pMeshData->m_rgVertices[iVertex].iMaterial = 0;
		pMeshData->m_rgVertices[iVertex].iFace = 0;
    }
    
    // for each face, add the face normal for each corner to the vertex node list array.
    // The index into the vertex node list array is just the vertex index, and a list of CVertexNodes
    // is built for the unique smoothing groups for that vertex.  Each CVertexNode holds the normal
    // at that vertex based on a particular smoothing group.  All CVertexNodes get an index, and
    // the list of CFaceIndices is a face list with the corners updated with the new expanded vertex
    // indices (CVertexNode index).
    //
    // Added: Now CVertexNodes each represent a unique smoothing group + texture coordinate set
    // for a vertex.
    for( iFace = 0; iFace < pMeshData->m_cFaces; iFace++ )
    {
        for( iPoint = 0; iPoint < 3; iPoint++ ) // vertex indices
        {
            iRawVertexIndex = pMesh->faces[iFace].v[iPoint];
            iTextureIndex = 0xFFFFFFFF;
            iSmoothingGroupIndex = pMesh->faces[iFace].smGroup;
            iMaterial = rgdwMeshMaterials[iFace];

            if ((pMesh->numTVerts > 0) 
                    && (NULL != pMesh->tvFace)
                    && ((int)pMesh->tvFace[iFace].t[iPoint] < pMesh->numTVerts) )
            {
                pMeshData->m_bTexCoordsPresent = TRUE;

                iTextureIndex = pMesh->tvFace[iFace].t[iPoint];
            }
            
            if (FALSE == rgbVertexReferencedArray[iRawVertexIndex])
            {
                // first reference to this vertex.
                rgbVertexReferencedArray[iRawVertexIndex] = TRUE;

                pMeshData->m_rgVertices[iRawVertexIndex].iSmoothingGroupIndex = iSmoothingGroupIndex;

                pMeshData->m_rgVertices[iRawVertexIndex].iTextureIndex = iTextureIndex;
                pMeshData->m_rgVertices[iRawVertexIndex].iMaterial = iMaterial;
				pMeshData->m_rgVertices[iRawVertexIndex].iFace= iFace;

                pMeshData->m_rgFaces[iFace].index[iPoint] = iRawVertexIndex;
            }
            else
            {
                // need to remember the index
                bFound = FindIdenticalVertex(pMeshData, iRawVertexIndex, iSmoothingGroupIndex, iTextureIndex, iMaterial, &iNewVertex);

                // if not found, then split out another vertex
                if (!bFound)
                {
                    // realloc if array too small
                    if (pMeshData->m_cVertices == cVerticesMax)
                    {
                        cVerticesMax = cVerticesMax * 2;
                        rgVerticesNew = new SVertexData[cVerticesMax];
                        if (rgVerticesNew == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto e_Exit;
                        }

                        memcpy(rgVerticesNew, pMeshData->m_rgVertices, sizeof(SVertexData) * pMeshData->m_cVertices);

                        delete []pMeshData->m_rgVertices;
                        pMeshData->m_rgVertices = rgVerticesNew;
                    }

                    // grab the next spot in the array
                    iNewVertex = pMeshData->m_cVertices;
                    pMeshData->m_cVertices += 1;

                    // setup point rep and wedge list
                    pMeshData->m_rgVertices[iNewVertex].iPointRep = iRawVertexIndex;

                    // set normal to 0 if first vertex in a new smoothing group, otherwise set to the same value as 
                    //   one of the other vertices in the same smoothing group
                    FindNormal(pMeshData, iRawVertexIndex, iSmoothingGroupIndex, pMeshData->m_rgVertices[iNewVertex].vNormal);

                    // link into wedge list of point rep
                    pMeshData->m_rgVertices[iNewVertex].iWedgeList = pMeshData->m_rgVertices[iRawVertexIndex].iWedgeList;
                    pMeshData->m_rgVertices[iRawVertexIndex].iWedgeList = iNewVertex;

                    // setup vertex info
                    pMeshData->m_rgVertices[iNewVertex].iSmoothingGroupIndex = iSmoothingGroupIndex;
                    pMeshData->m_rgVertices[iNewVertex].iTextureIndex = iTextureIndex;
                    pMeshData->m_rgVertices[iNewVertex].iMaterial = iMaterial;
					pMeshData->m_rgVertices[iNewVertex].iFace= iFace;

				}

                pMeshData->m_rgFaces[iFace].index[iPoint] = iNewVertex;
            }

            // add normal contribution to every vertex with this rawVertexIndex and
            // this smoothinggroup
            AddNormalContribution(pMeshData, iRawVertexIndex, iSmoothingGroupIndex, pMesh->getFaceNormal(iFace));
        }
    }


    for (iVertex = 0; iVertex < pMeshData->m_cVertices; iVertex++)
    {
		//###danhoro $NOTE    THIS IS TAKEN FROM THE MAX SAMPLES!!!
		RVertex *rvert= pMesh->getRVertPtr(pMeshData->m_rgVertices[iVertex].iPointRep);
		DWORD numNormals= rvert->rFlags & NORCT_MASK;
		if (rvert->rFlags & SPECIFIED_NORMAL) 
		{
			pMeshData->m_rgVertices[iVertex].vNormal = rvert->rn.getNormal();
		}
		else if (numNormals && pMeshData->m_rgVertices[iVertex].iSmoothingGroupIndex) 
		{
			// If there is only one vertex is found in the rn member.
			if (numNormals == 1 && rvert->ern == NULL) 
			{
				pMeshData->m_rgVertices[iVertex].vNormal = rvert->rn.getNormal();
			}
			else 
			{
				// If two or more vertices are there you need to step through them
				// and find the vertex with the same smoothing group as the current face.
				// You will find multiple normals in the ern member.
				for (unsigned int i = 0; i < numNormals; i++) 
				{
					if (rvert->ern[i].getSmGroup() & pMeshData->m_rgVertices[iVertex].iSmoothingGroupIndex) 
					{
						pMeshData->m_rgVertices[iVertex].vNormal = rvert->ern[i].getNormal();
					}
				}
			}
		}
		else 
		{
			// Get the normal from the Face if no smoothing groups are there
			pMeshData->m_rgVertices[iVertex].vNormal = pMesh->getFaceNormal(pMeshData->m_rgVertices[iVertex].iFace);
		}

        pMeshData->m_rgVertices[iVertex].vNormal = pMeshData->m_rgVertices[iVertex].vNormal.Normalize();
    }

e_Exit:
    delete[] rgbVertexReferencedArray;

    return hr;
}


/*
BOOL FindIdenticalPatchVertex
    (
    SPatchMeshData *pPatchMeshData, 
    DWORD iVertexIndex, 
    DWORD iTextureIndex,
    DWORD *piVertex
    )
{
    DWORD iHeadVertex;
    DWORD iCurVertex;

    // walk the wedge list to see if any of the other vertice have the same smoothing and texture index
    iHeadVertex = iVertexIndex;
    iCurVertex = iHeadVertex;
    do
    {
        // if identical requirements, then return true with the vertex index
        if (pPatchMeshData->m_rgVertices[iCurVertex].iTextureIndex == iTextureIndex)
        {
            *piVertex = iCurVertex;
            return TRUE;
        }

        // move to next element of wedge list and check to see if we wrapped (circular list)
        iCurVertex = pPatchMeshData->m_rgVertices[iCurVertex].iWedgeList;
    }
    while (iHeadVertex != iCurVertex);

    return FALSE;
}
*/


HRESULT GeneratePatchMeshData
    (
    PatchMesh *pPatchMesh,
    SPatchMeshData *pPatchMeshData,
	BOOL bParity
    )
{

    HRESULT hr = S_OK;
    BYTE *rgbVertexReferencedArray = NULL;
    DWORD cVerticesMax;
    //DWORD iRawVertexIndex;
    //DWORD iTextureIndex;
    DWORD iVertex;
    //DWORD iPoint;
    //DWORD iNewVertex;
    //BOOL bFound;
    //SPatchVertexData *rgVerticesNew;
	DWORD iCurVertex;
	LONG iPatch;
	//DWORD cPoints;
	PatchVec *pvPatchVecs;
	PatchTVert *pvPatchTVerts;
	TVPatch *pTVPatch;
    Patch *pPatch;
	DWORD *rgdwControl;
	//DWORD iInterior;
	float fParityInverter= 1.0f;

    assert(pPatchMesh != NULL);
    assert(pPatchMeshData != NULL);

    pPatchMeshData->m_bTexCoordsPresent = FALSE;
    pPatchMeshData->m_cPatches = pPatchMesh->numPatches;

	// UNDONE - need to refine
    cVerticesMax = pPatchMeshData->m_cPatches * 16;

    pPatchMeshData->m_rgVertices = new SPatchVertexData[cVerticesMax];
    pPatchMeshData->m_rgPatches = new SPatchData[pPatchMesh->numPatches];

    if ((pPatchMeshData->m_rgVertices == NULL) 
            || (pPatchMeshData->m_rgPatches == NULL))
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

	// initialize the verts being copied from the pPatchMesh->verts array
	for (iVertex = 0; iVertex < cVerticesMax; iVertex++)
	{
		pPatchMeshData->m_rgVertices[iVertex].iPointRep = iVertex;
		pPatchMeshData->m_rgVertices[iVertex].iWedgeList = iVertex;
	}

	pPatchMesh->computeInteriors();

	iCurVertex= 0;
    pvPatchVecs = pPatchMesh->vecs;
    for (iPatch = 0; iPatch < pPatchMesh->numPatches; iPatch++)
    {
        pPatch = &pPatchMesh->patches[iPatch];

		pPatchMesh->ChangePatchToCurvedMapping(iPatch);
		pPatch->computeInteriors(pPatchMesh);

		if (pPatch->type == PATCH_TRI)
        {
            // nControlIndices
            pPatchMeshData->m_rgPatches[iPatch].m_cControl = 10;

			//ok this is a little strange but this is how we store them
			// 6  
			// 5  7 
			// 4  9  8 
			// 3  2  1  0
			//the path looks like this
			//  
			// | \ 
			// |F_ \
			// |____S   
			//and to make it more complicated...
			//we have to reverse each row because 
			//of the parity difference to to RHS to LHS conversion

            rgdwControl = pPatchMeshData->m_rgPatches[iPatch].m_rgdwControl;

 			//no parity here
			rgdwControl[0] = iCurVertex;
			pPatchMeshData->m_rgVertices[iCurVertex].vPosition =pPatchMesh->verts[pPatch->v[0]].p;
			if(bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition *= -1.0f;
			iCurVertex += 1;

			rgdwControl[3] = iCurVertex; 
			if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pPatchMesh->verts[pPatch->v[2]].p;
 			else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pPatchMesh->verts[pPatch->v[1]].p;
            iCurVertex += 1;
			
			rgdwControl[6] = iCurVertex;
			if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pPatchMesh->verts[pPatch->v[1]].p;
  			else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pPatchMesh->verts[pPatch->v[2]].p;
           iCurVertex += 1;

 			rgdwControl[1] = iCurVertex;
            if(!bParity)
 				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[5]].p;//0
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[0]].p;//0
            iCurVertex += 1;
        
			rgdwControl[2] = iCurVertex;
            if(!bParity)
 				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[4]].p;//1
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[1]].p;//1
            iCurVertex += 1;
        
 			rgdwControl[4] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[3]].p;//2
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[2]].p;//2
            iCurVertex += 1;
        
			rgdwControl[5] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[2]].p;//3
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[3]].p;//3
            iCurVertex += 1;
        
			rgdwControl[7] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[1]].p;//4
			else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[4]].p;//4
            iCurVertex += 1;
        
			rgdwControl[8] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[0]].p;//5
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[5]].p;//5
            iCurVertex += 1;
        
            // UNDONE - is the correct way to get a single interior control
            //  point from 3ds max
            //no parity here
			rgdwControl[9] = iCurVertex;
  			pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[0]].p;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition += pvPatchVecs[pPatch->interior[1]].p;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition += pvPatchVecs[pPatch->interior[2]].p;
            pPatchMeshData->m_rgVertices[iCurVertex].vPosition /= 3;
            if(bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition*= -1.0f;
            iCurVertex += 1;
        }
        else if (pPatch->type == PATCH_QUAD)
        {
           // nControlIndices
            pPatchMeshData->m_rgPatches[iPatch].m_cControl = 16;

			//ok this is a little strange but this is how we store them
			// 6  7  8  9
			// 5  14 15 10
			// 4  13 12 11
			// 3  2  1  0
			//the path looks like this
			//  ________
			// |  ____F |
			// | |______|
			// |_________S
			//and to make it more complicated...
			//we have to reverse each row because 
			//of the parity difference to to RHS to LHS conversion

            rgdwControl = pPatchMeshData->m_rgPatches[iPatch].m_rgdwControl;
            
			rgdwControl[0] = iCurVertex;
			if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pPatchMesh->verts[pPatch->v[1]].p;
			else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pPatchMesh->verts[pPatch->v[1]].p;
            iCurVertex += 1;

			rgdwControl[3] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pPatchMesh->verts[pPatch->v[0]].p;
			else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pPatchMesh->verts[pPatch->v[2]].p;
            iCurVertex += 1;


			rgdwControl[6] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pPatchMesh->verts[pPatch->v[3]].p;
  			else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pPatchMesh->verts[pPatch->v[3]].p;
            iCurVertex += 1;
			
			rgdwControl[9] = iCurVertex;
			if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pPatchMesh->verts[pPatch->v[2]].p;
			else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pPatchMesh->verts[pPatch->v[0]].p;
            iCurVertex += 1;


			rgdwControl[1] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[1]].p;//0
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[2]].p;//0
            iCurVertex += 1;
        
			rgdwControl[2] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[0]].p;//1
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[3]].p;//1
            iCurVertex += 1;
        
			rgdwControl[4] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[7]].p;//2
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[4]].p;//2
            iCurVertex += 1;
        
			rgdwControl[5] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[6]].p;//3
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[5]].p;//3
            iCurVertex += 1;
        
			rgdwControl[7] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[5]].p;//4
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[6]].p;//4
            iCurVertex += 1;
        
			rgdwControl[8] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[4]].p;//5
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[7]].p;//5
            iCurVertex += 1;
        
			rgdwControl[10] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[3]].p;//6
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[0]].p;//6
            iCurVertex += 1;

			rgdwControl[11] = iCurVertex;
            if(!bParity)
 				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->vec[2]].p;//7
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->vec[1]].p;//7
            iCurVertex += 1;

			rgdwControl[12] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[1]].p;//0
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->interior[1]].p;//0
            iCurVertex += 1;

			rgdwControl[13] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[0]].p;//1
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->interior[2]].p;//1
            iCurVertex += 1;

			rgdwControl[14] = iCurVertex;
            if(!bParity)
 				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[3]].p;//2
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->interior[3]].p;//2
            iCurVertex += 1;

			rgdwControl[15] = iCurVertex;
            if(!bParity)
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = pvPatchVecs[pPatch->interior[2]].p;//3
            else
				pPatchMeshData->m_rgVertices[iCurVertex].vPosition = -1.0f*pvPatchVecs[pPatch->interior[0]].p;//3
            iCurVertex += 1;
        }
        else // undefined patch type
        {
            hr = E_INVALIDARG;
            goto e_Exit;
        }
    }

	// now record the initial number of vertices
	pPatchMeshData->m_cVerticesBeforeDuplication = iCurVertex;
	pPatchMeshData->m_cVertices = iCurVertex;

    rgbVertexReferencedArray = new BYTE[pPatchMeshData->m_cVertices];
	if (rgbVertexReferencedArray == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto e_Exit;
	}
	memset(rgbVertexReferencedArray, 0, sizeof(BYTE) * pPatchMeshData->m_cVertices);

	if (pPatchMesh->tvPatches.Count() >= 2) 
	{
		TVPatch* ptvp_channel0= pPatchMesh->tvPatches[0];
		TVPatch* ptvp_channel1= pPatchMesh->tvPatches[1];

		if(pPatchMesh->tvPatches[1] != NULL)
		{
			pPatchMeshData->m_bTexCoordsPresent = TRUE;

			pPatchMesh->computeInteriors();

			pvPatchTVerts= pPatchMesh->tVerts[1];
			iCurVertex= 0;//reset for texture pass
			for (iPatch = 0; iPatch < pPatchMesh->numPatches; iPatch++)
			{
				pPatch = &pPatchMesh->patches[iPatch];
				pTVPatch = &pPatchMesh->tvPatches[1][iPatch];

				if (pPatch->type == PATCH_TRI)
				{
  					//ok this is a little strange but this is how we store them
					// 6  
					// 5  7 
					// 4  9  8 
					// 3  2  1  0
					//the path looks like this
					//  
					// | \ 
					// |F_ \
					// |____S   
					//and to make it more complicated...
					//we have to reverse each row because 
					//of the parity difference to to RHS to LHS conversion
		 
  					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[0]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[1]].p;
					iCurVertex += 1;

					//no parity needed
					pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[2]].p;
					iCurVertex += 1;
					
					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[1]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[0]].p;
					iCurVertex += 1;

 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[5]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[0]].p;
					iCurVertex += 1;
		        
 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[4]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[1]].p;
					iCurVertex += 1;
		        
  					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[3]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[2]].p;
					iCurVertex += 1;
		        
  					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[2]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[3]].p;
					iCurVertex += 1;
		        
 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[1]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[4]].p;
					iCurVertex += 1;
		        
 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[0]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[5]].p;
					iCurVertex += 1;
		        
					// UNDONE - is the correct way to get a single interior control
					//  point from 3ds max
					//###danhoro $NOTE interiors tend to be -1 so invalid texture coordinate...must interpolate
					if(pTVPatch->interiors[0] >= 0
						&& pTVPatch->interiors[1] >= 0
						&& pTVPatch->interiors[2] >= 0)
					{
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->interiors[0]].p;
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord += pvPatchTVerts[pTVPatch->interiors[1]].p;
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord += pvPatchTVerts[pTVPatch->interiors[2]].p;
					}
					else
					{
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pPatchMeshData->m_rgVertices[0].vTexCoord;
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord += pPatchMeshData->m_rgVertices[3].vTexCoord;
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord += pPatchMeshData->m_rgVertices[6].vTexCoord;

					}
					pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord /= 3;
					iCurVertex += 1;
				}
				else if (pPatch->type == PATCH_QUAD)
				{
 					//ok this is a little strange but this is how we store them
					// 6  7  8  9
					// 5  14 15 10
					// 4  13 12 11
					// 3  2  1  0
					//the path looks like this
					//  ________
					// |  ____F |
					// | |______|
					// |_________S
					//and to make it more complicated...
					//we have to reverse each row because 
					//of the parity difference to to RHS to LHS conversion

 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[1]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[0]].p;
					iCurVertex += 1;

 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[0]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[1]].p;
					iCurVertex += 1;

 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[3]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[2]].p;
					iCurVertex += 1;
					
 					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[2]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->tv[3]].p;
					iCurVertex += 1;

  					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[1]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[0]].p;
					iCurVertex += 1;
		        
 					if(!bParity)
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[0]].p;
					else
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[1]].p;
					iCurVertex += 1;
		        
 					if(!bParity)
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[7]].p;
					else
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[2]].p;
					iCurVertex += 1;
		        
 					if(!bParity)
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[6]].p;
					else
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[3]].p;
					iCurVertex += 1;
		        
  					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[5]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[4]].p;
					iCurVertex += 1;
		        
 					if(!bParity)
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[4]].p;
					else
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[5]].p;
					iCurVertex += 1;
		        
  					if(!bParity)
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[3]].p;
					else
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[6]].p;
					iCurVertex += 1;

 					if(!bParity)
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[2]].p;
					else
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->handles[7]].p;
					iCurVertex += 1;

					

		

					if(pTVPatch->interiors[1] >= 0)
					{
 						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->interiors[1]].p;
					}
					else
					{
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.x= 
						0.25f*pPatchMeshData->m_rgVertices[4].vTexCoord.x 
						+0.75f*pPatchMeshData->m_rgVertices[11].vTexCoord.x;

						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.y= 
						0.25f*pPatchMeshData->m_rgVertices[8].vTexCoord.y 
						+0.75f*pPatchMeshData->m_rgVertices[1].vTexCoord.y;
					}
					iCurVertex += 1;

 					if(pTVPatch->interiors[0] >= 0)
					{
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->interiors[0]].p;
					}
					else
					{
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.x= 
						0.25f*pPatchMeshData->m_rgVertices[11].vTexCoord.x 
						+0.75f*pPatchMeshData->m_rgVertices[4].vTexCoord.x;

						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.y= 
						0.25f*pPatchMeshData->m_rgVertices[7].vTexCoord.y 
						+0.75f*pPatchMeshData->m_rgVertices[2].vTexCoord.y;
					}
					iCurVertex += 1;

					if(bParity)
					{
						//swap 0 & 1
						UVVert tmpVert= pPatchMeshData->m_rgVertices[iCurVertex-2].vTexCoord;
						pPatchMeshData->m_rgVertices[iCurVertex-2].vTexCoord= pPatchMeshData->m_rgVertices[iCurVertex-1].vTexCoord;
						pPatchMeshData->m_rgVertices[iCurVertex-1].vTexCoord= tmpVert;
					}

 					if(pTVPatch->interiors[3] >= 0)
					{ 
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->interiors[3]].p;
					}
					else
					{
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.x= 
						0.25f*pPatchMeshData->m_rgVertices[5].vTexCoord.x 
						+0.75f*pPatchMeshData->m_rgVertices[10].vTexCoord.x; 

						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.y= 
						0.25f*pPatchMeshData->m_rgVertices[2].vTexCoord.y 
						+0.75f*pPatchMeshData->m_rgVertices[7].vTexCoord.y; 
					}										
					iCurVertex += 1;
 
					if(pTVPatch->interiors[2] >= 0)
					{ 
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord = pvPatchTVerts[pTVPatch->interiors[2]].p;
					}
					else
					{
						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.x= 
						0.25f*pPatchMeshData->m_rgVertices[10].vTexCoord.x 
						+0.75f*pPatchMeshData->m_rgVertices[5].vTexCoord.x; 

						pPatchMeshData->m_rgVertices[iCurVertex].vTexCoord.y= 
						0.25f*pPatchMeshData->m_rgVertices[1].vTexCoord.y 
						+0.75f*pPatchMeshData->m_rgVertices[8].vTexCoord.y; 
					}				
					iCurVertex += 1;

					if(bParity)
					{
						//swap 0 & 1
						UVVert tmpVert= pPatchMeshData->m_rgVertices[iCurVertex-2].vTexCoord;
						pPatchMeshData->m_rgVertices[iCurVertex-2].vTexCoord= pPatchMeshData->m_rgVertices[iCurVertex-1].vTexCoord;
						pPatchMeshData->m_rgVertices[iCurVertex-1].vTexCoord= tmpVert;
					}


				}
				else // undefined patch type
				{
					hr = E_INVALIDARG;
					goto e_Exit;
				}
			}
		}
	}

e_Exit:
	delete []rgbVertexReferencedArray;

	return hr;
}