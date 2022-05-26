#ifndef __CONNECTIONS_H
#define __CONNECTIONS_H

class MaxVertexShader;
bool SaveConnections(ISave* pSave, nv_sys::INVParameterList* pParams);
nv_sys::INVParameterList* LoadConnections(MaxVertexShader* pVS, ILoad* pLoad);
bool ResolveConnections(nv_sys::INVParameterList* pDest, nv_sys::INVParameterList* pSource, bool bLoading);


#endif // __CONNECTIONS_H