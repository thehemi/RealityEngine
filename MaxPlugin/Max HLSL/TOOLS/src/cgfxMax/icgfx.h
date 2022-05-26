
#ifndef __ICGFX__H
#define __ICGFX__H

#include "iFnPub.h"


class ICgFXPlugin;


//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define CGFXPLUGIN_INTERFACE Interface_ID(0x667d32ce, 0x5bad024a)

#define GetICgFXPluginInterface(cd) \
			(ICgFXPlugin*)(cd)->GetInterface(CGFXPLUGIN_INTERFACE)

enum {  cgfxplugin_getnumtechniques,
		cgfxplugin_getnumparameters,
		cgfxplugin_getparameter,
		cgfxplugin_setparameter,
		cgfxplugin_getparametername,
		cgfxplugin_getparametersemantic
		};
//****************************************************************


class ICgFXPlugin : public FPMixinInterface 
{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

			FN_0(cgfxplugin_getnumtechniques, TYPE_INT, fnGetNumTechniques);
			FN_0(cgfxplugin_getnumparameters, TYPE_INT, fnGetNumParameters);
			FN_1(cgfxplugin_getparameter, TYPE_FPVALUE_BV, fnGetParameter, TYPE_INT);
			VFN_2(cgfxplugin_setparameter, fnSetParameter, TYPE_INT, TYPE_FPVALUE_BV);
			FN_1(cgfxplugin_getparametername, TYPE_STRING, fnGetParameterName, TYPE_INT);
			FN_1(cgfxplugin_getparametersemantic, TYPE_STRING, fnGetParameterSemantic, TYPE_INT);
/*
			VFN_0(lag_setreference, fnSetReference);
			VFN_0(lag_reset, fnReset);
			VFN_1(lag_addforce, fnAddForce,TYPE_INODE);
			VFN_1(lag_removeforce, fnRemoveForce,TYPE_INT);
			FN_0(lag_numbervertices,TYPE_INT, fnNumberVertices);

			VFN_2(lag_selectvertices, fnSelectVertices, TYPE_BITARRAY, TYPE_BOOL);
			FN_0(lag_getselectedvertices, TYPE_BITARRAY, fnGetSelectedVertices);

			FN_1(lag_getvertexweight, TYPE_FLOAT, fnGetVertexWeight, TYPE_INT);
			VFN_2(lag_setvertexweight, fnSetVertexWeight, TYPE_INT_TAB,TYPE_FLOAT_TAB);

			VFN_2(lag_setedgelist, fnSetEdgeList, TYPE_BITARRAY, TYPE_BOOL);
			FN_0(lag_getedgelist, TYPE_BITARRAY, fnGetEdgeList);

			VFN_2(lag_addspringselection, fnAddSingleSpringFromSelection,TYPE_INT,TYPE_BOOL);
			VFN_4(lag_addspring, fnAddSpring,TYPE_INT,TYPE_INT,TYPE_INT,TYPE_BOOL);
			*/


		END_FUNCTION_MAP


		FPInterfaceDesc* GetDesc();    // <-- must implement 
		
		//note functions that start with fn are to be used with maxscript since these expect 1 based indices
		virtual int		fnGetNumTechniques() = 0;
		virtual int		fnGetNumParameters() = 0;
		virtual FPValue fnGetParameter(int index) = 0;
		virtual void fnSetParameter(int index, FPValue Value) = 0;
		virtual TCHAR* fnGetParameterName(int index) = 0;
		virtual TCHAR* fnGetParameterSemantic(int index) = 0;

};





#endif // __ICGFX_H
