//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. =======
// C# Scripting System -- GUI System Wrappers
// Author: Mostafa Mohamed & Jeremy Stieglitz
//
//
//===============================================================================
#pragma	once
using namespace	System;
using namespace	System::Collections;
using namespace System::Reflection;
using namespace System::ComponentModel;
using namespace System::ComponentModel::Design;
using namespace System::ComponentModel::Design::Serialization;
using namespace System::Drawing;

using namespace stdcli::language;
#include "Helpers.h"
#include "ScriptingEngine.h"

namespace ScriptingSystem
{
    ref class MGUIControl;
    public delegate	void SSystem_DialogEvent(unsigned int Event, int ControlID, MGUIControl^ Control);
    void CALLBACK OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl );

     public ref class MGUITab
    {
        private public:
            CDXUTab * m_tab;
            MGUITab(CDXUTab * tab)
            {
                m_tab = tab;
            }
     };

    public ref class MGUITabPage
    {
        private public:
            DXUTPage * m_tabPage;
            bool needToDelete;

            MGUITabPage(DXUTPage * tabPage)
            {
                m_tabPage = tabPage;
                needToDelete = false;
            }
    public:
        MGUITabPage()
            {
                m_tabPage = new DXUTPage();
                needToDelete = true;
            }


        void Finalize()
        {
            if (needToDelete)
                delete m_tabPage;
        }

        property MGUITab^  Tab
        {
            MGUITab^ get()
            {
                return gcnew MGUITab(m_tabPage->m_Tab);
            }
        }
         property String^  Text
        {
            String^ get()
            {
                return Helpers::ToCLIString(m_tabPage->strText);
            }
            void set(String^ value)
            {
                wstring text = Helpers::ToCppStringw(value);
                wmemcpy(m_tabPage->strText,text.c_str(),256);
            }
        }
    };

     // MGUIControlsList----------------------------------------
    public ref class MGUITabPagesList //:  System::Collections::IList ?????????
    {
        private public:
            CDXUTTabPages * m_guiTabPages;

            MGUITabPagesList(CDXUTTabPages * tabPages)
            {
                m_guiTabPages = tabPages;
            }

    public: property bool IsReadOnly
            {
                bool get()
                {
                    return false;
                }
            }

    public: 

    public: int Add(Object^ value)
            {
                MGUITabPage^ page =(MGUITabPage^)value;
                return m_guiTabPages->m_Pages.Add( *page->m_tabPage);
            }

    public: property bool IsFixedSize
            {
                bool get()
                {
                    return false;
                }
            }


    public: property bool IsSynchronized
            {
                bool get()
                {
                    return true;
                }
            }

    public: property int Count
            {
                int get()
                {
                    return m_guiTabPages->m_Pages.GetSize();
                }
            }

    public: void CopyTo(Array^ array, int index)
            {
                //TODO
            }

    public: property Object^ SyncRoot
            {
                Object^ get()
                {
                    return nullptr;
                }
            }

    public: IEnumerator^ GetEnumerator()
            {
                return nullptr;
            }
    };

    // MGUIControl----------------------------------------
    public ref class MGUIControl 
    {
        private public:
            CDXUTControl * m_guiControl;

            MGUIControl(CDXUTControl * guiControl)
            {
                m_guiControl = guiControl;
            }
    public:
        property Point^ Location
        {
            Point^ get()
            {
                return gcnew Point(m_guiControl->m_x,m_guiControl->m_y);
            }
            void set(Point^ value)
            {
                m_guiControl->SetLocation(value->X,value->Y );
            }
        }

		property int ControlID
        {
            int get()
            {
                return m_guiControl->GetID();
            }
            void set(int value)
            {
                m_guiControl->SetID(value);
            }
        }

        property System::Drawing::Size^ Size
        {
            System::Drawing::Size^ get()
            {
                return gcnew System::Drawing::Size(m_guiControl->m_width,m_guiControl->m_height);
            }
            void set(System::Drawing::Size^ value)
            {
                m_guiControl->SetSize(value->Width ,value->Height );
            }
        }
    };

     // MGUITabPages----------------------------------------
    public ref class MGUITabPages: MGUIControl 
    {
        private public:
            MGUITabPagesList^ m_pagesList;
            MGUITabPages(CDXUTTabPages * guiControl) : MGUIControl((CDXUTControl*)guiControl)
            {
                m_pagesList = gcnew MGUITabPagesList((CDXUTTabPages*)m_guiControl);
            }
    public:
        MGUITabPages() : MGUIControl((CDXUTControl*)new CDXUTTabPages())
        {
            m_pagesList = gcnew MGUITabPagesList((CDXUTTabPages*)m_guiControl);
        }

        property MGUITabPagesList^ TabPages
        {
            MGUITabPagesList^ get()
            {
                return m_pagesList;
            }
        }

        bool IsSelected(MGUITab^ tab)
	     {
             return ((CDXUTTabPages*)m_guiControl)->isSelected(tab->m_tab);
        }

        void AddTabPage(String^ text, int width)
        {
            ((CDXUTTabPages*)m_guiControl)->AddTabPage(Helpers::ToCppStringw(text).c_str(),width);
        }

        void AddControlToTabPage(int i,  MGUIControl^ control)
        {
            ((CDXUTTabPages*)m_guiControl)->AddControlToTabPage(i,control->m_guiControl);
        }

        void ResetToFirstTab()
        {
            ((CDXUTTabPages*)m_guiControl)->ResetToFirstTab();
        }
    };


    // MGUIEditBox----------------------------------------
    public ref class MGUIEditBox : MGUIControl 
    {
        private public:

            MGUIEditBox(CDXUTEditBox * guiControl) : MGUIControl((CDXUTControl*)guiControl)
            {}
    public:
        MGUIEditBox() : MGUIControl((CDXUTControl*)new CDXUTEditBox())
        {}

        property String^  Text
        {
            String^ get()
            {
                return Helpers::ToCLIString(((CDXUTEditBox*)m_guiControl)->GetText());
            }
            void set(String^ value)
            {
                wstring text = Helpers::ToCppStringw(value);
                ((CDXUTEditBox*)m_guiControl)->SetText((WCHAR*)text.c_str());
            }
        }

         property int BorderWidth
        {
            int get()
            {
                return ((CDXUTEditBox*)m_guiControl)->GetBorderWidth();
            }
            void set(int value)
            {
                ((CDXUTEditBox*)m_guiControl)->SetBorderWidth(value);
            }
        }
    };

    // MGUIListBox----------------------------------------
    public ref class MGUIListBox : MGUIControl 
    {
        private public:

            MGUIListBox(CDXUTListBox * guiControl) : MGUIControl((CDXUTControl*)guiControl)
            {}
    public:
        MGUIListBox() : MGUIControl((CDXUTControl*)new CDXUTListBox())
        {}

		property String^ SelectedItemText
		{
			String^ get()
			{
				if(((CDXUTListBox*)m_guiControl)->GetSize() && ((CDXUTListBox*)m_guiControl)->GetSelectedIndex() > -1)
					return Helpers::ToCLIString(ToAnsi(((CDXUTListBox*)m_guiControl)->GetSelectedItem()->strText));
				else
					return Helpers::ToCLIString("");					
			}
		}

		property int SelectedItemData
		{
			int get()
			{
				if(((CDXUTListBox*)m_guiControl)->GetSize() && ((CDXUTListBox*)m_guiControl)->GetSelectedIndex() > -1)
					return (int)((CDXUTListBox*)m_guiControl)->GetSelectedItem()->pData;
				else
					return 0;					
			}
		}

		property int SelectedIndex
		{
			int get()
			{
				if(((CDXUTListBox*)m_guiControl)->GetSize())
					return ((CDXUTListBox*)m_guiControl)->GetSelectedIndex();
				else
					return -1;					
			}
			void set(int value)
			{
				if(value < ((CDXUTListBox*)m_guiControl)->GetSize())
					((CDXUTListBox*)m_guiControl)->SelectItem(value);					
			}
		}

        property int Style
        {
            int get()
            {
                return ((CDXUTListBox*)m_guiControl)->GetStyle();
            }
            void set(int value)
            {
                ((CDXUTListBox*)m_guiControl)->SetStyle(value);
            }
        }

        property int ListSize
        {
            int get()
            {
                return ((CDXUTListBox*)m_guiControl)->GetSize();
            }
        }

        property int ScrollbarWidth
        {
            void set(int value)
            {
                ((CDXUTListBox*)m_guiControl)->SetScrollBarWidth(value);
            }
        }

        void SetBorder( int border, int margin )
        {
            ((CDXUTListBox*)m_guiControl)->SetBorder(border,margin);
        }

        bool AddItem( String^ text )
        {
            if (SUCCEEDED(((CDXUTListBox*)m_guiControl)->AddItem( Helpers::ToCppStringw(text).c_str(), NULL)))
                return true;
            return false;
        }

        bool InsertItem( int index, String^ text )
        {
            if (SUCCEEDED(((CDXUTListBox*)m_guiControl)->InsertItem( index, Helpers::ToCppStringw(text).c_str(), NULL)))
                return true;
            return false;
        }

        void RemoveItem( int Index )
        {
            ((CDXUTListBox*)m_guiControl)->RemoveItem( Index );
        }

        void RemoveItemByText(String^ text)
        {
            ((CDXUTListBox*)m_guiControl)->RemoveItemByText((WCHAR*)Helpers::ToCppStringw(text).c_str());
        }

        void RemoveAllItems()
        {
            ((CDXUTListBox*)m_guiControl)->RemoveAllItems();
        }
    };


    // MGUISlider----------------------------------------
    public ref class MGUISlider : MGUIControl 
    {
        private public:

            MGUISlider(CDXUTSlider * guiControl) : MGUIControl((CDXUTControl*)guiControl)
            {}
    public:
        MGUISlider() : MGUIControl((CDXUTControl*)new CDXUTSlider())
        {}

        property int Value
        {
            int get()
            {
                return ((CDXUTSlider*)m_guiControl)->GetValue();
            }
            void set(int value)
            {
                ((CDXUTSlider*)m_guiControl)->SetValue(value);
            }
        }

        property int Min
        {
            int get()
            {
                return ((CDXUTSlider*)m_guiControl)->GetMin();
            }
            void set(int value)
            {
                ((CDXUTSlider*)m_guiControl)->SetRange(value,((CDXUTSlider*)m_guiControl)->GetMax());
            }
        }

        property int Max
        {
            int get()
            {
                return ((CDXUTSlider*)m_guiControl)->GetMax();
            }
            void set(int value)
            {
                ((CDXUTSlider*)m_guiControl)->SetRange(((CDXUTSlider*)m_guiControl)->GetMin(),value);
            }
        }

        property float UnitScale
        {
            float get()
            {
                return ((CDXUTSlider*)m_guiControl)->m_UnitScale;
            }
            void set(float value)
            {
                ((CDXUTSlider*)m_guiControl)->m_UnitScale = value;
            }
        }
    };

    // MGUISlider----------------------------------------
    public ref class MGUIScrollBar : MGUIControl 
    {
        private public:

            MGUIScrollBar(CDXUTScrollBar * guiControl) : MGUIControl((CDXUTControl*)guiControl)
            {}
    public:
        MGUIScrollBar() : MGUIControl((CDXUTControl*)new CDXUTScrollBar())
        {}

        property int PageSize
        {
            int get()
            {
                return ((CDXUTScrollBar*)m_guiControl)->GetPageSize();
            }
            void set(int value)
            {
                ((CDXUTScrollBar*)m_guiControl)->SetPageSize(value);
            }
        }

        property int TrackPos
        {
            int get()
            {
                return ((CDXUTScrollBar*)m_guiControl)->GetTrackPos();
            }
            void set(int value)
            {
                ((CDXUTScrollBar*)m_guiControl)->SetTrackPos(value);
            }
        }

        property int TrackStart
        {
            int get()
            {
                return ((CDXUTScrollBar*)m_guiControl)->GetTrackStart();
            }
            void set(int value)
            {
                ((CDXUTScrollBar*)m_guiControl)->SetTrackRange(((CDXUTScrollBar*)m_guiControl)->GetTrackStart(),value);
            }
        }

        property int TrackEnd
        {
            int get()
            {
                return ((CDXUTScrollBar*)m_guiControl)->GetTrackEnd();
            }
            void set(int value)
            {
                ((CDXUTScrollBar*)m_guiControl)->SetTrackRange(value,((CDXUTScrollBar*)m_guiControl)->GetTrackEnd());
            }
        }
    };

    // MGUIStatic----------------------------------------
    public ref class MGUIStatic : MGUIControl 
    {
        private public:

            MGUIStatic(CDXUTStatic * guiControl) : MGUIControl((CDXUTControl*)guiControl)
            {}
    public:
        MGUIStatic() : MGUIControl((CDXUTControl*)new CDXUTStatic())
        {}

        property String^  Text
        {
            /* String^ get()
            {
            return Helpers::ToCLIString(((CDXUTStatic*)m_guiControl)->GetText());
            }*/
            void set(String^ value)
            {
                wstring text = Helpers::ToCppStringw(value);
                ((CDXUTStatic*)m_guiControl)->SetText((WCHAR*)text.c_str());
            }
        }
    };

	// MGUIPictureLabel----------------------------------------
    public ref class MGUIPictureLabel : MGUIControl 
    {
        private public:

            MGUIPictureLabel(CPictureLabel* guiControl) : MGUIControl((CDXUTControl*)guiControl)
            {}
    public:
        MGUIPictureLabel() : MGUIControl((CDXUTControl*)new CPictureLabel(-1, 0, 0, 64, 64, "", D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA))
        {}
        property String^ TextureFilename
        {
			String^ get()
            {
            return Helpers::ToCLIString(((CPictureLabel*)m_guiControl)->GetTextureFilename());
            }
            void set(String^ value)
            {
                ((CPictureLabel*)m_guiControl)->ChangeTexture(Helpers::ToCppString(value));
            }
        }
		property MBlendMode SourceBlend
        {
			MBlendMode get()
            {
				return (MBlendMode)(int)((CPictureLabel*)m_guiControl)->m_Source;
            }
            void set(MBlendMode value)
            {
				((CPictureLabel*)m_guiControl)->m_Source = (D3DBLEND)(int)value;
            }
        }
		property MBlendMode DestBlend
        {
			MBlendMode get()
            {
				return (MBlendMode)(int)((CPictureLabel*)m_guiControl)->m_Dest;
            }
            void set(MBlendMode value)
            {
				((CPictureLabel*)m_guiControl)->m_Dest = (D3DBLEND)(int)value;
            }
        }
    };

    // MGUIButton----------------------------------------
    public ref class MGUIButton : MGUIStatic 
    {
        private public:

            MGUIButton(CDXUTButton * guiControl) : MGUIStatic((CDXUTStatic*)guiControl)
            {}
    public:
        MGUIButton() : MGUIStatic((CDXUTStatic*)new CDXUTButton())
        {}
    };

    // MGUIComboBox----------------------------------------
    public ref class MGUIComboBox : MGUIButton 
    {
        private public:

            MGUIComboBox(CDXUTComboBox * guiControl) : MGUIButton((CDXUTButton*)guiControl)
            {}
    public:
        MGUIComboBox() : MGUIButton((CDXUTButton*)new CDXUTComboBox())
        {
        }

        property int DropHeight
        {
            void set(int value)
            {
                ((CDXUTComboBox*)m_guiControl)->SetDropHeight(value);
            }
        }

        property int ScrollbarWidth
        {
            void set(int value)
            {
                ((CDXUTComboBox*)m_guiControl)->SetScrollBarWidth(value);
            }
        }

        bool AddItem( String^ text )
        {
            if (SUCCEEDED(((CDXUTComboBox*)m_guiControl)->AddItem( Helpers::ToCppStringw(text).c_str(), NULL)))
                return true;
            return false;
        }
		bool AddItem( String^ text, int userData)
        {
            if (SUCCEEDED(((CDXUTComboBox*)m_guiControl)->AddItem( Helpers::ToCppStringw(text).c_str(), (void*)(int)userData)))
                return true;
            return false;
        }

				property String^ SelectedItemText
		{
			String^ get()
			{
				if(((CDXUTComboBox*)m_guiControl)->GetNumItems() && ((CDXUTComboBox*)m_guiControl)->GetSelectedIndex() > -1)
					return Helpers::ToCLIString(ToAnsi(((CDXUTComboBox*)m_guiControl)->GetSelectedItem()->strText));
				else
					return Helpers::ToCLIString("");					
			}
		}

		property int SelectedItemData
		{
			int get()
			{
				if(((CDXUTComboBox*)m_guiControl)->GetNumItems() && ((CDXUTComboBox*)m_guiControl)->GetSelectedIndex() > -1)
					return (int)((CDXUTComboBox*)m_guiControl)->GetSelectedItem()->pData;
				else
					return 0;					
			}
		}

		property int SelectedIndex
		{
			int get()
			{
				if(((CDXUTComboBox*)m_guiControl)->GetNumItems())
					return ((CDXUTComboBox*)m_guiControl)->GetSelectedIndex();
				else
					return -1;					
			}
			void set(int value)
			{
				if(value < ((CDXUTComboBox*)m_guiControl)->GetNumItems())
					((CDXUTComboBox*)m_guiControl)->SetSelectedByIndex(value);					
			}
		}

		void SetSelectedByData(int data)
		{
			((CDXUTComboBox*)m_guiControl)->SetSelectedByData((void*)(int)data);
		}

        void RemoveItem( int Index )
        {
            ((CDXUTComboBox*)m_guiControl)->RemoveItem( Index );
        }

        void RemoveAllItems()
        {
            ((CDXUTComboBox*)m_guiControl)->RemoveAllItems();
        }

        property int NumItems
        {
            int get()
            {
                return ((CDXUTComboBox*)m_guiControl)->GetNumItems();
            }
        }

        bool SetSelectedByText( String^ text )
        {
            if (SUCCEEDED(((CDXUTComboBox*)m_guiControl)->SetSelectedByText((WCHAR*)Helpers::ToCppStringw(text).c_str())))
                return true;
            return false;
        }

    };



    // MGUICheckBox----------------------------------------
    public ref class MGUICheckBox : MGUIButton 
    {
        private public:

            MGUICheckBox(CDXUTCheckBox * guiControl) : MGUIButton((CDXUTButton*)guiControl)
            {}
    public:
        MGUICheckBox() : MGUIButton((CDXUTButton*)new CDXUTCheckBox())
        {}

        property virtual bool Checked
        {
            bool get()	
            {
                return ((CDXUTCheckBox*)m_guiControl)->GetChecked();
            }
            void set(bool value)
            {
                ((CDXUTCheckBox*)m_guiControl)->SetChecked(value);
            }
        }
    };

    // MGUICheckBox----------------------------------------
    public ref class MGUIRadioButton: MGUICheckBox 
    {
        private public:

            MGUIRadioButton(CDXUTRadioButton * guiControl) : MGUICheckBox((CDXUTCheckBox*)guiControl)
            {}
    public:
        MGUIRadioButton() : MGUICheckBox((CDXUTCheckBox*)new CDXUTRadioButton())
        {}

        property virtual bool Checked
        {
            bool get()	
            {
                return ((CDXUTCheckBox*)m_guiControl)->GetChecked();
            }
            void set(bool value)
            {
                ((CDXUTRadioButton*)m_guiControl)->SetChecked(value, true);
            }
        }

        property unsigned int ButtonGroup
        {
            unsigned int get()	
            {
                return ((CDXUTRadioButton*)m_guiControl)->GetButtonGroup();
            }
            void set(unsigned int value)
            {
                ((CDXUTRadioButton*)m_guiControl)->SetButtonGroup(value);
            }
        }
    };




    // MGUIControlsList----------------------------------------
    public ref class MGUIControlsList //:  System::Collections::IList ?????????
    {
        private public:
            CDXUTDialog * m_guiDialog;

            MGUIControlsList(CDXUTDialog * dialog)
            {
                m_guiDialog = dialog;
            }

    public: property bool IsReadOnly
            {
                bool get()
                {
                    return false;
                }
            }

    public: 



        /*    property int X[int index]
        {
        int get(int index)
        {
        // TODO:  Add k.this getter implementation
        return nullptr;
        }
        void set(int index,int value)
        {
        // TODO:  Add k.this setter implementation
        }
        }*/

    public: void RemoveAt(int index)
            {
                m_guiDialog->RemoveAt(index);
            }

    public: void Insert(int index, Object^ value)
            {
                MGUIControl^ control =(MGUIControl^)value;
                m_guiDialog->InsertControl(index, control->m_guiControl);
            }

    public: void Remove(Object^ value)
            {
                MGUIControl^ control =(MGUIControl^)value;
                m_guiDialog->RemoveControl(control->m_guiControl->GetID());
            }

    public: bool Contains(Object^ value)
            {
                MGUIControl^ control =(MGUIControl^)value;
                return m_guiDialog->Contains(control->m_guiControl);
            }

    public: void Clear()
            {
                m_guiDialog->RemoveAllControls();
            }

    public: int IndexOf(Object^ value)
            {
                MGUIControl^ control =(MGUIControl^)value;
                return m_guiDialog->IndexOfControl(control->m_guiControl);
            }

    public: int Add(Object^ value)
            {
                MGUIControl^ control =(MGUIControl^)value;
                return m_guiDialog->AddControl(control->m_guiControl);
            }

    public: property bool IsFixedSize
            {
                bool get()
                {
                    return false;
                }
            }


    public: property bool IsSynchronized
            {
                bool get()
                {
                    return true;
                }
            }

    public: property int Count
            {
                int get()
                {
                    return m_guiDialog->GetControlsCount();
                }
            }

    public: void CopyTo(Array^ array, int index)
            {
                //TODO
            }

    public: property Object^ SyncRoot
            {
                Object^ get()
                {
                    return nullptr;
                }
            }

    public: IEnumerator^ GetEnumerator()
            {
                return nullptr;
            }
    };

    // GUIDialog----------------------------------------
    //[Designer(typeid<Artificial::GUIDesigner::DialogDesigner>, typeid<System::ComponentModel::Design::IRootDesigner>)]
    public ref class MGUIDialog : public Component
    {
        private public:
            MGUIControlsList^ m_controlsList;
            CDXUTDialog * m_guiDialog;
            class DialogEventHandler * m_eventHandler;
            bool needToDelete;
            int myIndex;

            MGUIDialog(CDXUTDialog * guiDialog)
            {
                ScriptingEngine::s_dialogs->Add(ScriptingEngine::s_dialogIndex,this);
                m_guiDialog = guiDialog;
                m_guiDialog->managedIndex = ScriptingEngine::s_dialogIndex;
                myIndex = ScriptingEngine::s_dialogIndex;
                ScriptingEngine::s_dialogIndex++;
                m_controlsList = gcnew MGUIControlsList(guiDialog);
                needToDelete = false;
                m_guiDialog->SetCallback(::OnEvent);
            }
    public:
		static ArrayList^ MGUIDialogs = gcnew ArrayList();
        MGUIDialog()
        {
			MGUIDialogs->Add(this);
            ScriptingEngine::s_dialogs->Add(ScriptingEngine::s_dialogIndex,this);
            m_guiDialog = new CDXUTDialog();
            m_guiDialog->managedIndex = ScriptingEngine::s_dialogIndex;
            ScriptingEngine::s_dialogIndex++;
            m_controlsList = gcnew MGUIControlsList(m_guiDialog);
            needToDelete = true;
            m_guiDialog->SetCallback(::OnEvent);
            m_guiDialog->SetCaptionText(L"Dialog");
        }

        void Finalize()
        {
			MGUIDialogs->Remove(this);
            ScriptingEngine::s_dialogs->Remove(myIndex);
            if (needToDelete)
                SAFE_DELETE( m_guiDialog );
        }

        //PROPERTIES
    public:
        event SSystem_DialogEvent^ DialogEvent;
        property MGUIControlsList^ Controls
        {
            MGUIControlsList^ get()
            {
                return m_controlsList;
            }
        }

        property String^  Text
        {
            void set(String^ value)
            {
                wstring text = Helpers::ToCppStringw(value);
                m_guiDialog->SetCaptionText((WCHAR*)text.c_str());
            }
        }
		property bool Minimized
		{
			bool get()
            {
                return m_guiDialog->IsMinimized();
            }
		}
		property bool CaptionEnabled
		{
			bool get()
            {
                return m_guiDialog->GetCaptionEnabled();
            }
			void set(bool value)
            {     
                m_guiDialog->EnableCaption(value);
            }
		}
        property Point^ Location
        {
            Point^ get()
            {
                return gcnew Point(m_guiDialog->GetX(),m_guiDialog->GetY());
            }
            void set(Point^ value)
            {
                m_guiDialog->SetLocation(value->X,value->Y );
            }
        }

		property bool DrawBackground
		{
			bool get()
            {
                return m_guiDialog->m_bDisplayBG;
            }
            void set(bool value)
            {     
                m_guiDialog->m_bDisplayBG = value;
            }
		}

		property bool Draggable
		{
			bool get()
            {
                return m_guiDialog->bDraggable;
            }
            void set(bool value)
            {     
                m_guiDialog->bDraggable = value;
            }
		}

        property System::Drawing::Size^ Size
        {
            System::Drawing::Size^ get()
            {
                return gcnew System::Drawing::Size(m_guiDialog->GetWidth(),m_guiDialog->GetHeight());
            }
            void set(System::Drawing::Size^ value)
            {
                m_guiDialog->SetSize(value->Width ,value->Height );
            }
        }

        property bool EnableCaption
        {
            bool get()
            {
                return m_guiDialog->GetCaptionEnabled();
            }
            void set(bool value)
            {     
                m_guiDialog->EnableCaption(value);
            }
        }

        property bool EnableMouseInput
        {
            void set(bool value)
            {     
                m_guiDialog->EnableMouseInput(value);
            }
        }

			public: 
			void Render(float elapsedTime){m_guiDialog->OnRender(elapsedTime);}
			virtual void OnRender(){}
			MGUIControl^ GetControl(int ControlID)
			{
						MGUIControl^ control = nullptr;
						CDXUTControl* pControl = m_guiDialog->GetControl(ControlID);
						if(pControl)
						{
                        switch (pControl->GetType())
                        {
                        case DXUT_CONTROL_BUTTON:
                            control = gcnew MGUIButton((CDXUTButton*)pControl);
                            break;
                        case DXUT_CONTROL_EDITBOX:
                            control = gcnew MGUIEditBox((CDXUTEditBox*)pControl);
                            break;
                        case DXUT_CONTROL_SLIDER:
                            control = gcnew MGUISlider((CDXUTSlider*)pControl);
                            break;
                        case DXUT_CONTROL_SCROLLBAR:
                            control = gcnew MGUIScrollBar((CDXUTScrollBar*)pControl);
                            break;
                        case DXUT_CONTROL_STATIC:
                            control = gcnew MGUIStatic((CDXUTStatic*)pControl);
                            break;
                        case DXUT_CONTROL_CHECKBOX:
                            control = gcnew MGUICheckBox((CDXUTCheckBox*)pControl);
                            break;
                        case DXUT_CONTROL_RADIOBUTTON:
                            control = gcnew MGUIRadioButton((CDXUTRadioButton*)pControl);
                            break;
                        case DXUT_CONTROL_COMBOBOX:
                            control = gcnew MGUIComboBox((CDXUTComboBox*)pControl);
                            break;
                        case DXUT_CONTROL_LISTBOX:
                            control = gcnew MGUIListBox((CDXUTListBox*)pControl);
                            break;
                        }
						}
						return control;
			}

            private public:
                void OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
                {
                    if (DialogEvent != nullptr)
                    {
                        MGUIControl^ control;
                        switch (pControl->GetType())
                        {
                        case DXUT_CONTROL_BUTTON:
                            control = gcnew MGUIButton((CDXUTButton*)pControl);
                            break;
                        case DXUT_CONTROL_EDITBOX:
                            control = gcnew MGUIEditBox((CDXUTEditBox*)pControl);
                            break;
                        case DXUT_CONTROL_SLIDER:
                            control = gcnew MGUISlider((CDXUTSlider*)pControl);
                            break;
                        case DXUT_CONTROL_SCROLLBAR:
                            control = gcnew MGUIScrollBar((CDXUTScrollBar*)pControl);
                            break;
                        case DXUT_CONTROL_STATIC:
                            control = gcnew MGUIStatic((CDXUTStatic*)pControl);
                            break;
                        case DXUT_CONTROL_CHECKBOX:
                            control = gcnew MGUICheckBox((CDXUTCheckBox*)pControl);
                            break;
                        case DXUT_CONTROL_RADIOBUTTON:
                            control = gcnew MGUIRadioButton((CDXUTRadioButton*)pControl);
                            break;
                        case DXUT_CONTROL_COMBOBOX:
                            control = gcnew MGUIComboBox((CDXUTComboBox*)pControl);
                            break;
                        case DXUT_CONTROL_LISTBOX:
                            control = gcnew MGUIListBox((CDXUTListBox*)pControl);
                            break;
                        }
                        if (control!=nullptr)
                            DialogEvent(nEvent,nControlID,control);
                    }
                }
    };

    //EVENT HANDLER
    void CALLBACK OnEvent( UINT nEvent, int nControlID, CDXUTControl* pControl )
    {
        if (pControl->m_pDialog->managedIndex != -1)
            ((MGUIDialog^)ScriptingEngine::s_dialogs[pControl->m_pDialog->managedIndex])->
            OnEvent( nEvent, nControlID,  pControl );
    }

	
    // GUI WINDOW -----------------------------------------------
    public ref class MGUIWindow : Component
    {
        private public:
            CGUIWindow * m_guiWindow;
            bool needToDelete;

            MGUIWindow(CGUIWindow * guiWindow)
            {
                m_guiWindow = guiWindow;
                needToDelete = false;
            }
    public:
        MGUIWindow(MGUIDialog^ dialog, String^ configIdentifier)
        {
            m_guiWindow = new CGUIWindow(dialog->m_guiDialog , Helpers::ToCppString(configIdentifier));
            needToDelete = true;
        }

        void Finalize()
        {
            if (needToDelete)
                SAFE_DELETE(m_guiWindow);
        }
    public:
		void BringToTop()
		{
			GUISystem::Instance()->BringToTop(m_guiWindow);
		}
        property MGUIDialog^  Dialog
        {
            MGUIDialog^ get()	
            {
                return gcnew MGUIDialog(m_guiWindow->m_Dialog);
            }
            void set(MGUIDialog^ value)
            {
                m_guiWindow->m_Dialog = value->m_guiDialog;
            }
        }

        property bool Enabled
        {
            bool get()	
            {
                return m_guiWindow->GetEnabled();
            }
            void set(bool value)
            {
                m_guiWindow->SetEnabled(value);
            }
        }

		property bool IsDesktop
        {
            bool get()	
            {
                return m_guiWindow->GetDesktop();
            }
            void set(bool value)
            {
                m_guiWindow->SetDesktop(value);
            }
        }

        property bool  Visible
        {
            bool get()	
            {
                return m_guiWindow->GetVisible();
            }
            void set(bool value)
            {
                m_guiWindow->SetVisible(value);
            }
        }

        property bool  DesktopWindow
        {
            bool get()	
            {
                return m_guiWindow->GetDesktopWindow();
            }
            void set(bool value)
            {
                m_guiWindow->SetDesktopWindow(value);
            }
        }
    };

	public ref class MVideoSettingsDialog : MGUIDialog
	{
	public:
		MVideoSettingsDialog() : MGUIDialog(GUISystem::Instance()->GetVideoSettingsDialog())
        {
        }
	};

    public ref class MGUISystem
    {
    public:

        static MGUIWindow^ RegisterAsWindow(MGUIDialog ^ dialog)
        {
            return gcnew MGUIWindow(GUISystem::Instance()->RegisterAsWindow(dialog->m_guiDialog));
        }
		static void SetMessageBoxWindow(MGUIWindow^ window)
		{
			GUISystem::Instance()->SetMessageBoxWindow(window->m_guiWindow);
		}
        static property bool DesktopVisible
        {
            bool get()	
            {
                return GUISystem::Instance()->DesktopVisible();
            }
            void set(bool value)
            {
                GUISystem::Instance()->ShowDesktop(value);
            }
        }
        static property bool DisplayGUI
        {
            bool get()	
            {
                return GUISystem::Instance()->GetDisplayGUI();
            }
            void set(bool value)
            {
                GUISystem::Instance()->SetDisplayGUI(value);
            }
        }
		static void HandleVideoSettingsCallback(unsigned int Event, int ControlID, MGUIControl^ Control)
		{
			GUISystem::Instance()->OnVideoSettingsEvent(Event, ControlID, Control->m_guiControl);
		}
		static void DisplayMessageBox(String^ title, String^ text, float blockTimeSeconds)
		{
			if(blockTimeSeconds > 0)
				GUISystem::Instance()->DoMessageBoxBlocking(Helpers::ToCppString(title), Helpers::ToCppString(text), blockTimeSeconds, NULL);
			else
			{
				GUISystem::Instance()->CloseMessageBoxBlocking();
				GUISystem::Instance()->DoMessageBox(Helpers::ToCppString(title), Helpers::ToCppString(text),-1,-1);
			}
		}
		static void EndMessageBoxBlocking()
		{
			GUISystem::Instance()->CloseMessageBoxBlocking();
		}
    };
}