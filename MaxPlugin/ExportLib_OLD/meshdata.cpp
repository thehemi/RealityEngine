//-----------------------------------------------------------------------------
// File: MeshData.cpp
//
// Desc: Functions used to massage mesh data into a format useable in X Files
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "stdafx.h"
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
