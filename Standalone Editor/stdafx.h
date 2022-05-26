// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <windows.h>

// Undefine things that fuck up .NET
#undef MessageBox
#undef GetObject

extern HINSTANCE hInstance;
#include "..\shared\shared.h"
#include "..\Stub\Stub.h"
#include "..\shared\vector.h"

#include "..\EngineInc\Engine.h"
#include "..\EngineInc\classmap.h"
#include "..\EngineInc\Editor.h"
#include "..\EngineInc\Profiler.h"

#include "..\EngineSrc\Serializers\OBJ.h"
#include "..\EngineSrc\Serializers\XFile.h"
#include "..\EngineSrc\Compiler\Compiler.h"

#include "..\EngineInc\SkyController.h"


using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace PropertyBags;
using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;


// New data types
// TODO: Need a 'TypeConverter' of type Float3 to get this to work.
ref class Float3
{
public:

	property float X  { 
		float get()  
		{ 
			return x; 
		} 
		void set(float value)  
		{ 
			x=value; 
		} 
	}
	property float Y  { 
		float get()  
		{ 
			return y; 
		} 
		void set(float value)  
		{ 
			y=value; 
		} 
	}
	property float Z  { 
		float get()  
		{ 
			return z; 
		} 
		void set(float value)  
		{ 
			z=value; 
		} 
	}
	Float3(float fX, float fY, float fZ){ X = fX; Y = fY; Z = fZ; }
private:
	float x,y,z;
}; 

///
ref class MatWrapper
{
public: class Material* mat;
public: System::String^ file;
public: bool bRefreshMe;
public: MatWrapper(){ bRefreshMe = false; }
};

/// <summary>
/// Handles propertybags and editorvars
/// </summary>
ref class PropertyHelper
{
public:
	vector<vector<EditorVar>*>* currentVars;
	PropertyBags::PropertyBag^  class1;

	PropertyHelper()
	{
		currentVars = new vector<vector<EditorVar>*>;

		class1 = gcnew PropertyBags::PropertyBag();
		// Register events
		class1->GetValue += gcnew PropertySpecEventHandler(this,&PropertyHelper::GetValue);
		class1->SetValue += gcnew PropertySpecEventHandler(this,&PropertyHelper::SetValue);
	}

	void UpdateVars(vector<EditorVar>& vars);
	void GetValue(Object^ sender, PropertySpecEventArgs^ e);
	void SetValue(Object^ sender, PropertySpecEventArgs^ e);
};


// <summary>
// Helper Function
// </summary>
std::string ToCppString(System::String ^ str);
System::String^ STLToManaged(std::string stl);


 /*
 [DllImport("EngineD")]
 extern "C"  int __cdecl GetValue(MulticastDelegate^ del);
 
 ref class Managed
 {
 public:
     static void Callback(int i)
     {        
         Console::WriteLine("void Managed::Callback()");
     }
 };
 
 typedef void (*FunctionPointer)(int i);
 delegate void MyDelegate(int i);

 ref class Pinnable
 {
 public: int pinned;
 public: MyDelegate^ func;
 };
*/


// FIXME: Get this working as a way for C++ to call MC++
	/*
	[StructLayoutAttribute( LayoutKind::Sequential, CharSet = CharSet::Ansi )]
	ref struct Delegate_Wrapper
	{
		[MarshalAsAttribute(UnmanagedType::FunctionPtr)]
		MyDelegate^  _theDelegate;
	};
	*/









ref class ThumbNailButton : public System::Windows::Forms::Button
{
public: System::Windows::Forms::Form^ formRef;
public: System::String^ fileName;
public: float LastTimeClicked;

public: ThumbNailButton(System::String^ fileName,
			System::String^ pictureName,
			System::Drawing::Point PositionIn,
			System::Drawing::Size SizeIn,
			System::Windows::Forms::SplitterPanel ^ formAddTo,
			System::Windows::Forms::Form^ MainFormIn)
		{			
			formAddTo->SuspendLayout();

			LastTimeClicked=GSeconds;

			formRef=MainFormIn;

			try
			{
				this->BorderColor = System::Drawing::Color::Black;
				this->BorderSize = 10;
				this->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
				
				if(fileName!=nullptr)
				this->Image = System::Drawing::Image::FromFile(fileName);					
				
				this->Location = PositionIn;
				this->Name = pictureName;
				this->Size = SizeIn;
				this->TabIndex = 0;	
				this->Text = pictureName;
				this->ForeColor = System::Drawing::Color::Green;

				this->Click += gcnew System::EventHandler(this, &ThumbNailButton::ButtonClick);

				formAddTo->Controls->Add(this);
			}
			catch (System::Exception^ e)
			{
				// out of memory exception thrown for bad format
				System::Windows::Forms::MessageBox::Show("File not found or invalid format");
			}

			formAddTo->ResumeLayout(false);
		};

public:  System::Void RemoveFromForm(System::Windows::Forms::SplitterPanel^ formAddTo)
		 {
			 formAddTo->SuspendLayout();
			 formAddTo->Controls->Remove(this);
			 formAddTo->ResumeLayout(false);
		 };


public:  System::Void Remove()
		 {
			 formRef->SuspendLayout();
			 formRef->Controls->Remove(this);
			 formRef->ResumeLayout(false);
		 };		 

public:	System::Void ClearToBlack()
		{
			this->BorderColor = System::Drawing::Color::Black;
		};

public: bool IsSet()
		{
			if(this->BorderColor==System::Drawing::Color::Blue)return true;
			else return false;
		};

private: System::Void ButtonClick(System::Object^  sender, System::EventArgs^  e);
		 
};

/// <summary>
/// Callback for compiling, to allow window refreshing and cancelling
/// </summary>
class CompilerCallbackEx : CompilerCallback
{
public:
    CompilerCallbackEx(){ bCancel = false; }
    bool bCancel;
    virtual bool OnCallback();
};

extern CompilerCallbackEx callback;