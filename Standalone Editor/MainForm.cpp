#include "stdafx.h"
#include "MainForm.h"
#include <windows.h>

using namespace StandaloneEditor;

HINSTANCE hInstance;
CompilerCallbackEx callback;

[STAThread] 
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    Application::EnableVisualStyles();
	::hInstance = hInstance;
	System::Threading::Thread::CurrentThread->ApartmentState = System::Threading::ApartmentState::STA;
	gcnew MainForm();
	// No App::Run, we handle message loop in ctor
	//Application::Run()
	return 0;
}


namespace PropGrid
{
 /// <summary>
 /// CustomClass implements ICustomTypeDescriptor and derivesExpandableObjectConverter
 /// This class can be instantiated and properties to this class canbe added to it dynamically.
 /// Use AddProperty(string propName, System.Type propType, stringpropDesc, object propValue) to add a property.
 /// </summary>

 //[TypeConverter(typeof(ExpandableObjectConverter))]
 public ref class CustomClass : Component, ICustomTypeDescriptor
 {
  /// <summary>
  /// Constructor of CustomClass which initializes the DataTable.
  /// </summary>
 public: CustomClass()
  {
  }

 };
}
