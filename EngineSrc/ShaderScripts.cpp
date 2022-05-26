//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// Support for DirectX Standard Annotations & Semantics, including scripting
//
// See: "Using Standard Semantics and Standard Annotations" in SDK
// See: "Standard Annotations and Semantics - Dave Aronson.ppt"
//
//
// Author: Tim Johnson
//==============================================================================
#include "stdafx.h"



//-----------------------------------------------------------------------------
// Process script globals
// See "Standard Annotation Scripting Syntax"
//-----------------------------------------------------------------------------
void Shader::ProcessGlobals()
{
    m_Flags = 0;

    D3DXHANDLE global = effect->GetParameterBySemantic(NULL,"StandardsGlobal");
    if(!global)
        return;

    D3DXHANDLE script = effect->GetAnnotationByName(global,"Script");
    if(script)
    {
        string str = GetString(script);

        if(str.find("ReadsColorBuffer") != -1)
            m_Flags |= ReadsColorBuffer;

		if(str.find("ReadsLDRBuffer") != -1)
            m_Flags |= ReadsLDRBuffer;

		if(str.find("ReadsGlobalColorBuffer") != -1)
            m_Flags |= ReadsGlobalColorBuffer;
    }
}
