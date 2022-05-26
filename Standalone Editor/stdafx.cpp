// stdafx.cpp : source file that includes just the standard includes
// Standalone Editor.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "AssetBrowser.h"
#include "MainForm.h"
using namespace StandaloneEditor;
 

//Converts a System::String to a std::string
//This code assumes that you have used the following namespace:
// using namespace System::Runtime::InteropServices;
std::string ToCppString(String ^ managed)
{
    /*
	System::IntPtr
		ptr(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str));
	std::string ret(static_cast<const char *>(static_cast<void *>(ptr)));
	System::Runtime::InteropServices::Marshal::FreeCoTaskMem(ptr);
	return(ret);
    */
    //get a pointer to an array of ANSI chars
    char *chars = (char*) Marshal::StringToHGlobalAnsi(managed).ToPointer(); 

    //assign the array to an STL string
    std::string stl = chars; 
    //free the memory used by the array
    //since the array is not managed, it will not be claimed by the garbage collector
    Marshal::FreeHGlobal((System::IntPtr)chars); 
    return stl;
}

System::String^ STLToManaged(std::string stl) 
{
    //the c_str() function gets a char array from the STL string,
    //but the PtrToStringAnsi function wants a int array, so it gets casted
	return Marshal::PtrToStringAnsi((System::IntPtr) ((int *)stl.c_str()));
} 


void PropertyHelper::UpdateVars(vector<EditorVar>& vars)
{
	currentVars->clear();
	currentVars->push_back(&vars);
	class1->Properties->Clear();

	// Add script vars
	for(int i=0;i<vars.size();i++)
	{
		Type ^ type;
		if(vars[i].type == EditorVar::BOOL)
			type = typeid<bool>;
		else if(vars[i].type == EditorVar::STRING || vars[i].type == EditorVar::FILENAME || vars[i].type == EditorVar::TEXTURE)
			type = typeid<String>;
		else if(vars[i].type == EditorVar::INT)
			type = typeid<int>;
		else if(vars[i].type == EditorVar::FLOAT)
			type = typeid<float>;
		else if(vars[i].type == EditorVar::COLOR)
			type = typeid<Color>;
		else if(vars[i].type == EditorVar::FLOAT3)
			type = __typeof(Float3);
		else if(vars[i].type == EditorVar::FLOAT4)
			type = __typeof(Color);

		PropertySpec^ spec = gcnew PropertySpec(gcnew String(vars[i].name.c_str()), type, gcnew String(vars[i].category.c_str()),gcnew String(vars[i].desc.c_str()));

		if(vars[i].type == EditorVar::FILENAME || vars[i].type == EditorVar::TEXTURE)
		{
			Type^ ed   = typeid<System::Windows::Forms::Design::FileNameEditor>;
			Type^ conv = typeid<System::Drawing::Design::UITypeEditor>;

			spec->EditorTypeName = ed->AssemblyQualifiedName;
			spec->TypeName = ed->AssemblyQualifiedName;
		}
		class1->Properties->Add(spec);
	}
}


// <summary>
// Sets UI values from source params
// FIXME: This gets called a lot of times, maybe should use index compare instead of string compare
// </summary>
void PropertyHelper::GetValue(Object^ sender, PropertySpecEventArgs^ e)
{
	if(currentVars->size()==0)
		return;

	vector<EditorVar>& vars = *(*currentVars)[0];
	for(int i=0;i<vars.size();i++)
	{
		if(gcnew String(vars[i].name.c_str()) == e->Property->Name)
		{
			if(vars[i].type == EditorVar::BOOL){
				e->Value = *(bool*)vars[i].data;
			}
			else if(vars[i].type == EditorVar::INT){
				e->Value = *(int*)vars[i].data;
			}
			else if(vars[i].type == EditorVar::FLOAT){
				e->Value = *(float*)vars[i].data;
			}
			else if(vars[i].type == EditorVar::FLOAT3){
				Vector* v = (Vector*)vars[i].data;
				e->Value = gcnew Float3(v->x,v->y,v->z);
			}
			else if(vars[i].type == EditorVar::FLOAT4){
				FloatColor* sCol = (FloatColor*)vars[i].data;
				sCol->Clamp();
				e->Value = Color::FromArgb(sCol->a*255,sCol->r*255,sCol->g*255,sCol->b*255);
			}
			else if(vars[i].type == EditorVar::COLOR){
				FloatColor* sCol = (FloatColor*)vars[i].data;
				sCol->Clamp();
				
				e->Value = Color::FromArgb(sCol->a*255,sCol->r*255,sCol->g*255,sCol->b*255);

			}
			else if(vars[i].type == EditorVar::STRING || vars[i].type == EditorVar::FILENAME){
				e->Value = gcnew String((*(string*)vars[i].data).c_str());
			} 
			else if(vars[i].type == EditorVar::TEXTURE){
				// Just add texture name without path
				String^ name = STLToManaged(((Texture*)vars[i].data)->filename.c_str());
				if(name->Contains("\\"))
                    name = name->Substring(name->IndexOf("\\")+1);
				e->Value = name;
			}

		}
	}
}

// <summary>
// Updates source values from UI values
// </summary>
void PropertyHelper::SetValue(Object^ sender, PropertySpecEventArgs^ e)
{
	// We update ALL registered params, this allows for multi-selection application
	for(int j=0;j<currentVars->size();j++){
		vector<EditorVar>& vars = *(*currentVars)[j];
		for(int i=0;i<vars.size();i++)
		{
			if(gcnew String(vars[i].name.c_str()) == e->Property->Name)
			{
				if(vars[i].type == EditorVar::BOOL)
					*(bool*)vars[i].data = *reinterpret_cast<bool^>(e->Value);
				else if(vars[i].type == EditorVar::STRING)
				{
					*(string*)vars[i].data = ToCppString(reinterpret_cast<String^>(e->Value));
				}
				else if(vars[i].type == EditorVar::FILENAME)
				{
					*(string*)vars[i].data = ToCppString(reinterpret_cast<String^>(e->Value));
					// Semi-intelligent filename parsing
					if(AssetBrowser::me->selectedMat && AsLower(*(string*)vars[i].data).find(".fx") != -1)
					{
                        AssetBrowser::me->selectedMat->mat->Initialize(AssetBrowser::me->selectedMat->mat->m_ShaderName.c_str(),"",true);
						AssetBrowser::me->MaterialChanged(nullptr,nullptr);
					}
				}
				else if(vars[i].type == EditorVar::FLOAT)
					*(float*)vars[i].data = *reinterpret_cast<float^>(e->Value);
				else if(vars[i].type == EditorVar::INT)
					*(int*)vars[i].data = *reinterpret_cast<int^>(e->Value);
				else if(vars[i].type == EditorVar::FLOAT3){
					Vector* v = (Vector*)vars[i].data;
					Float3^ f = reinterpret_cast<Float3^>(e->Value);
					v->x = f->X;
					v->y = f->Y;
					v->z = f->Z;
				}
				else if(vars[i].type == EditorVar::COLOR){
					Color^ col = reinterpret_cast<Color^>(e->Value);
					FloatColor* sCol = (FloatColor*)vars[i].data;
					sCol->a = float(col->A)/255.f;
					sCol->r = float(col->R)/255.f;
					sCol->g = float(col->G)/255.f;
					sCol->b = float(col->B)/255.f;
				}
				else if(vars[i].type == EditorVar::FLOAT4){
					Color^ col = reinterpret_cast<Color^>(e->Value);
					FloatColor* sCol = (FloatColor*)vars[i].data;
					sCol->a = float(col->A)/255.f;
					sCol->r = float(col->R)/255.f;
					sCol->g = float(col->G)/255.f;
					sCol->b = float(col->B)/255.f;
				}
				else if(vars[i].type == EditorVar::TEXTURE){
					Texture* tex = ((Texture*)vars[i].data);
					string newName = ToCppString(reinterpret_cast<String^>(e->Value));
					string oldName = tex->filename;
					if(oldName.find_last_of("\\") != -1)
						oldName = oldName.substr(oldName.find_last_of("\\")+1);
					// New texture?
					if(oldName != newName)
					{
                        tex->Load(ToCppString(reinterpret_cast<String^>(e->Value)).c_str());
					}
				}
			}
		}
	}

    /// Refresh if a param is updated
    if(AssetBrowser::me && AssetBrowser::me->selectedMat && AssetBrowser::me->selectedMat->mat)
        AssetBrowser::me->selectedMat->mat->Refresh();
}


System::Void ThumbNailButton::ButtonClick(System::Object^  sender, System::EventArgs^  e)
		 {
			
			 //Asset Clears All to black so set it blue
			 ((AssetBrowser^) formRef)->ThumbChanged();

			 System::Windows::Forms::Button^ ButtonIn = (System::Windows::Forms::Button^)sender;			
			 ButtonIn->BorderColor = System::Drawing::Color::Blue;

			 if((GSeconds-LastTimeClicked) < 0.5f)
			 	((AssetBrowser^) formRef)->addPrefabToolStripButton_Click(nullptr, nullptr);
						 
			 LastTimeClicked=GSeconds;
		 }

bool CompilerCallbackEx::OnCallback()
{
    MainForm::me->UpdateCompilerProgress();
    System::Windows::Forms::Application::DoEvents();
    return bCancel;
}