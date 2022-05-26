//--------------------------------------------------------------------------------------
/// Reality Builder. Copyright Artificial Studios (c)2004 
///
/// TODO: Ref counting for material lib - However, don't want mat deleted if not used,
/// so do we really need ref counting?
///
/// TODO: Resolve any outdated include/exclude lists when items are deleted
/// Could result in crash for lights, water, etc
//--------------------------------------------------------------------------------------

#pragma once
#include "SplashForm.h"
#include "SelectForm.h"
#include "AssetBrowser.h"
#include "OBJExportForm.h"
#include "SplitForm.h"
#include "MeshSettings.h"

using namespace ScriptingSystem;
using namespace System::Diagnostics;

#define PICONVERSION 57.29577951

namespace StandaloneEditor
{


    /// <summary> 
    /// Summary for MainForm
    ///
    /// WARNING: If you change the name of this class, you will need to change the 
    ///          'Resource File Name' property for the managed resource compiler tool 
    ///          associated with all .resx files this class depends on.  Otherwise,
    ///          the designers will not be able to interact properly with localized
    ///          resources associated with this form.
    /// </summary>
    public ref class MainForm : public System::Windows::Forms::Form
    {	
    private:
       
        bool m_bRunning;
        int m_currentGizmoMode;
        PropertyHelper^ propHelper;
        Editor*	editor;
        int curSelectedIndex ;

    public: 
        static MainForm^ me;
        static vector<ModelFrame*>* selectedMeshes;
        // Helper forms
        SelectForm^   selectForm;
        AssetBrowser^ assetBrowser;
        OBJExportForm^ OBJForm;
        SplitForm^ splitForm;
        MeshSettings^ meshSettings;

    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator5;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator3;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator6;



    private: System::Windows::Forms::ToolStripMenuItem^  mergeToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveSelectedToolStripMenuItem;
    private: System::Windows::Forms::ToolStripButton^  toolStripSeparator1;
    private: System::Windows::Forms::ToolStripButton^  toolStripButton2;


    private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  visitOnlineHelpToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;

    private: System::Windows::Forms::ToolStripButton^  toolStripButton4;
    private: System::Windows::Forms::ToolStripButton^  orbitViewToolStripButton;
    private: System::Windows::Forms::ToolStripButton^  toolStripAssetBrowser;
             public private: System::Windows::Forms::ToolStripButton^  changeActor;
    private: System::Windows::Forms::ToolStripMenuItem^  editWorldPropertiesToolStripMenuItem;

    private: System::Windows::Forms::ToolStripMenuItem^  hideSelectedToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  freezeSelectedToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  unHideAllToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  unFreezeAllToolStripMenuItem;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator12;
    private: System::Windows::Forms::ToolStripMenuItem^  hideUnselectedToolStripMenuItem;
    private: System::Windows::Forms::ToolStrip^  toolStrip2;
    private: System::Windows::Forms::ToolStripButton^  sTGToolStripButton;
    private: System::Windows::Forms::ToolStripTextBox^  sTGEditBoxToolStripButton;
    private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip1;
    private: System::Windows::Forms::ToolStripMenuItem^  testToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  hideUnSelectedToolStripMenuItem1;
    private: System::Windows::Forms::ToolStripMenuItem^  unhideAllToolStripMenuItem1;
    private: System::Windows::Forms::ToolStripMenuItem^  freezeSelectedToolStripMenuItem1;
    private: System::Windows::Forms::ToolStripMenuItem^  unFreezeAllToolStripMenuItem1;
    private: System::Windows::Forms::ToolStripMenuItem^  modifiersToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  flipNormalsToolStripMenuItem;
    private: System::Windows::Forms::SplitContainer^  splitContainer3;
    private: System::Windows::Forms::RichTextBox^  richTextBox1;
    private: System::Windows::Forms::TextBox^  statisticsBox;
    private: System::Windows::Forms::SplitContainer^  splitContainer4;
    private: System::Windows::Forms::Label^  label2;

    private: System::Windows::Forms::PropertyGrid^  renderProps;
    private: System::Windows::Forms::Button^  buttonRemoveBlockers;
    private: System::Windows::Forms::Label^  label4;
    private: System::Windows::Forms::ListBox^  listBoxScene;
    private: System::Windows::Forms::Label^  label3;
    private: System::Windows::Forms::Button^  buttonAddBlockers;
    private: System::Windows::Forms::ListBox^  listBoxBlockers;
    private: System::Windows::Forms::Panel^  panel1;
    private: System::Windows::Forms::Label^  label5;
    private: System::Windows::Forms::Label^  label1;
    private: System::Windows::Forms::Button^  button2;
    private: System::Windows::Forms::Button^  button1;
    private: System::Windows::Forms::Button^  button4;
    private: System::Windows::Forms::Button^  button3;
    private: System::Windows::Forms::Label^  label7;
    private: System::Windows::Forms::Label^  label6;
    private: System::Windows::Forms::ListBox^  listExclusion;
    private: System::Windows::Forms::ListBox^  listSceneMeshes1;
    private: System::Windows::Forms::CheckBox^  checkIsExculsion;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator7;
    private: System::Windows::Forms::ToolStripComboBox^  toolStripComboBox1;
    private: System::Windows::Forms::Panel^  panel2;
    private: System::Windows::Forms::Panel^  panel3;
    private: System::Windows::Forms::LinkLabel^  linkLabel1;
    private: System::Windows::Forms::Label^  label9;
    private: System::Windows::Forms::Label^  label8;
    private: System::Windows::Forms::ToolStripMenuItem^  exportSelectedToolStripMenuItem;
    private: System::Windows::Forms::SaveFileDialog^  exportFileDialog;
    private: System::Windows::Forms::ToolStripMenuItem^  viewToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  wireframeToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  bboxToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  occToolStripMenuItem;

    private: System::Windows::Forms::ToolStripSeparator^  toolStripMenuItem4;
    private: System::Windows::Forms::TabPage^  tabPage3;
    private: System::Windows::Forms::Panel^  panel4;
    private: System::Windows::Forms::Button^  button9;
    private: System::Windows::Forms::Button^  button8;
    private: System::Windows::Forms::Button^  button7;
    private: System::Windows::Forms::ListBox^  selectedListBox;
    private: System::Windows::Forms::Button^  button6;
    private: System::Windows::Forms::Button^  button5;
    private: System::Windows::Forms::ComboBox^  selectedListComboBox;
    private: System::Windows::Forms::Label^  label10;
    private: System::Windows::Forms::Button^  clearListButton;
    private: System::Windows::Forms::ToolStripMenuItem^  invertMeshToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  regenerateTangentsNormalsLightingToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  redoToolStripMenuItem;
    private: System::Windows::Forms::TabPage^  tabPage4;
    private: System::Windows::Forms::TextBox^  statsText;
    private: System::Windows::Forms::ToolStripMenuItem^  exportSelectedAsPrefabsToolStripMenuItem;
    private: System::Windows::Forms::ComboBox^  comboSubmesh;
private: System::Windows::Forms::ComboBox^  comboSubmat;
private: System::Windows::Forms::Label^  labelSubmat;
private: System::Windows::Forms::ToolStripButton^  consoleToolStripButton;
private: System::Windows::Forms::ToolStripTextBox^  consoleToolStripTextBox;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator4;
private: System::Windows::Forms::ToolStripMenuItem^  centerPivotToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  combineSelectedPrefabsToolStripMenuItem;
private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
private: System::Windows::Forms::ToolStripMenuItem^  splitMeshToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  bakePRTLightingToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  breakMeshintosubelementsToolStripMenuItem;




             bool bInGameDLL;

    public: static void Callback(int i)
            {
                Application::DoEvents();
            }

    public: MainForm(void)
            {
                me = this;
                selectedMeshes = NULL;
                curSelectedIndex=-1;
                propHelper = gcnew PropertyHelper();
                bInGameDLL = false;
                InitializeComponent();
                propertyGrid1->SelectedObject = propHelper->class1;
                renderProps->SelectedObject = propHelper->class1;
                m_bRunning = false;
                m_currentGizmoMode=-1;
                editor = Editor::Instance();


                this->TopMost = false;

                // Load other forms app uses
                selectForm   = gcnew SelectForm();
                selectForm->Owner = this;
                selectForm->TopMost = true;

                assetBrowser = gcnew AssetBrowser();
                assetBrowser->Owner = this;
                assetBrowser->TopMost = true;

                OBJForm = gcnew OBJExportForm();
                OBJForm->Owner = this;
                OBJForm->TopMost = true;

                splitForm = gcnew SplitForm();
                splitForm->Owner = this;
                splitForm->TopMost = true;

                meshSettings = gcnew MeshSettings();
                meshSettings->Owner = this;
                meshSettings->TopMost = true;

                // Events
                this->splitContainer2->Panel1->BackColor = Color::Transparent;
                this->consoleToolStripTextBox->KeyDown += gcnew KeyEventHandler(this, &MainForm::ConsoleCommand);


                this->splitContainer2->Panel1->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &MainForm::TopView_KeyDown);


                this->splitContainer2->Panel1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::TopView_MouseMove);
                this->splitContainer2->Panel1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::TopView_MouseDown);
                this->splitContainer2->Panel1->MouseUp   += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::TopView_MouseUp);

                this->boxX->Leave += gcnew EventHandler(this, &MainForm::X_Leave);
                this->boxY->Leave += gcnew EventHandler(this, &MainForm::Y_Leave);
                this->boxZ->Leave += gcnew EventHandler(this, &MainForm::Z_Leave);

                this->sTGEditBoxToolStripButton->Leave += gcnew EventHandler(this, &MainForm::STG_Leave);

                // Closing events
                this->Closing += gcnew System::ComponentModel::CancelEventHandler(this, &MainForm::Main_OnClose);
                //this->Unload += gcnew System::EventHandler(this, &MainForm::Main_OnUnload);

                //
                // TIM: My attempt to stop the background redrawing over the render window, which caused flickering
                // TODO: Re-render on repaint
                //
                System::Windows::Forms::ControlStyles param = 
                    System::Windows::Forms::ControlStyles::AllPaintingInWmPaint|
                    System::Windows::Forms::ControlStyles::UserPaint
                    //| System::Windows::Forms::ControlStyles::DoubleBuffer
                    | System::Windows::Forms::ControlStyles::Opaque ;
                Object^ o1 = param;
                Object^ o2 = true;
                stdcli::language::array<Object^> ^ params1    = gcnew stdcli::language::array<Object^>(2){
                    reinterpret_cast<Object^>(o1), reinterpret_cast<Object^>(o2)};
                    Type^ t = typeid<System::Windows::Forms::SplitterPanel>;
                    System::Reflection::Binder^ b;
                    t->InvokeMember("SetStyle", Reflection::BindingFlags::NonPublic  |Reflection::BindingFlags::Instance | 
                        Reflection::BindingFlags::InvokeMethod, b,splitContainer2->Panel1, params1);

                    //
                    // Load MRU list
                    //
                    ResetCurrentDirectory();
                    ConfigFile file;
                    string name = "MRU.ini";
                    if(FindMedia(name,"System"))
                    {
                        file.Load(name);
                        for(int i=0;i<10;i++)
                        {
                            if(file.KeyExists("MRU"+ToStr(i)))
                            {
                                string str = file.GetString("MRU"+ToStr(i));
                                if(str.length())
                                    AddMRU(gcnew String(str.c_str()));
                            }
                        } 
                    }
                    else
                        file.Create(name);

                    //
                    // Application Loop using DoEvents()
                    // Far more responsive than OnIdle(), though less efficient
                    //
                    this->Hide();
                    while(true)
                    {
                        Application::DoEvents();

                        if(m_bRunning)
                        {
                            UpdateUI();
                            // Update the engine during idle time (no messages are waiting)
                            if(!bInGameDLL)
                            {
                                bInGameDLL = true;
                                Game_Tick();
                                bInGameDLL = false;
                            }
                            else
                                LogPrintf("Skipping tick (recursive). This shouldn't happen!");
                        }
                        else if(!LibsInitialized())
                        {
                            SplashForm^ splash = gcnew SplashForm();

                            splash->Show();
                            splash->Refresh();

                            // Load Engine
                            m_bRunning = true;
                            editor->SetEditorMode(true);
                            LoadLibraries((HWND)this->Handle.ToInt32(),(HWND)this->splitContainer2->Panel1->Handle.ToInt32(),hInstance,"");	 
                            editor->SetEditorMode(true);

                            //
                            // NOTE: Some issue with CRT heap sharing - Deallocating in MC++
                            // Can cause an exception in release with IDE. Why??? Using all CRT DLLs!
                            std::vector<std::string> classes = editor->GetAllClasses();

                            for (int i=0;i<classes.size();i++)
                                actorsCombo->Items->Add(gcnew String(classes[i].c_str()));

                            // Hide Splash, Show main window
                            splash->Hide();
                            Show();
                            this->splitContainer2->Panel1->Focus();

                            occToolStripMenuItem->Checked = RenderDevice::Instance()->GetOcclusionTesting();
                        }
                        else
                            break;
                        ///if((HWND)this->Handle.ToInt32() != GetForegroundWindow() && (HWND)mainMenu->Handle.ToInt32() != GetForegroundWindow() && !editor->m_bCompiling)
                        //	Sleep(200);
                    }
                    Game_Shutdown();
                    this->Close();
            }

    protected:
        void Dispose(Boolean disposing)
        {
            m_bRunning = false;
            if (disposing && components)
            {
                components->Dispose();
            }
            __super::Dispose(disposing);
        }


    private: System::Windows::Forms::ToolStripMenuItem^  editToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  undoToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  deleteToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  buildToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  buildAllToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  buildFastToolStripMenuItem;
    private: System::Windows::Forms::OpenFileDialog^  importMeshDialog;
    private: System::Windows::Forms::ToolStripMenuItem^  importMeshToolStripMenuItem;

    private: System::Windows::Forms::ToolStrip^  toolStrip1;
    private: System::Windows::Forms::ToolStripButton^  toolStripButton1;
    private: System::Windows::Forms::ToolStripComboBox^  actorsCombo;
    private: System::Windows::Forms::ToolStripSeparator^  insertToolBarSep;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
    private: System::Windows::Forms::ToolStripButton^  slowToolStripButton;
    private: System::Windows::Forms::ToolStripButton^  mediumToolStripButton;
    private: System::Windows::Forms::ToolStripButton^  fastToolStripButton;
    private: System::Windows::Forms::SplitContainer^  splitContainer1;
    private: System::Windows::Forms::SplitContainer^  splitContainer2;


    private: System::Windows::Forms::ToolStrip^  toolStrip3;
    private: System::Windows::Forms::ToolStripLabel^  xToolStripButton;
    private: System::Windows::Forms::ToolStripLabel^  toolStripLabel2;
    private: System::Windows::Forms::ToolStripLabel^  toolStripLabel1;
    private: System::Windows::Forms::ToolStripTextBox^  boxX;
    private: System::Windows::Forms::ToolStripTextBox^  boxY;
    private: System::Windows::Forms::ToolStripTextBox^  boxZ;
    private: System::Windows::Forms::TabPage^  tabPage1;
    private: System::Windows::Forms::PropertyGrid^  propertyGrid1;
    private: System::Windows::Forms::TabControl^  tabPanelRight;
    private: System::Windows::Forms::ToolStripMenuItem^  buildSelectedToolStripMenuItem;
    private: System::Windows::Forms::StatusStripPanel^  statusStripPanel1;
    private: System::Windows::Forms::TabPage^  tabPage2;














    private: System::Windows::Forms::SaveFileDialog^  saveFileDialog;
    private: System::Windows::Forms::ToolStripMenuItem^  openToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  gameToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  playToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  reloadSkyToolStripMenuItem;

    private: System::Windows::Forms::ToolStripButton^  addActorButton;
    private: System::Windows::Forms::ToolStripMenuItem^  selectObjectsToolStripMenuItem;
    private: System::Windows::Forms::ToolStripButton^  takeScreenshot;










    private: System::Windows::Forms::ToolStripMenuItem^  buildSelectedFastToolStripMenuItem;
    private: System::Windows::Forms::ImageList ^  ToolBarImages;
    private: System::Windows::Forms::RaftingContainer^  leftRaftingContainer;
    private: System::Windows::Forms::RaftingContainer^  rightRaftingContainer;
    private: System::Windows::Forms::RaftingContainer^  topRaftingContainer;
    private: System::Windows::Forms::RaftingContainer^  bottomRaftingContainer;
    private: System::Windows::Forms::ToolStrip^  mainToolbar;
    private: System::Windows::Forms::StatusStrip^  statusBar;

    private: System::Windows::Forms::ToolStripButton^  saveButton;
    private: System::Windows::Forms::ToolStrip^  toolsToolbar;
    private: System::Windows::Forms::ToolStripButton^  moveButton;
    private: System::Windows::Forms::ToolStripButton^  rotateButton;
    private: System::Windows::Forms::ToolStripButton^  scaleButton;
    private: System::Windows::Forms::ToolStripMenuItem^  newMenuButton;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripMenuItem1;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripMenuItem2;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripMenuItem3;
    private: System::Windows::Forms::ToolStripMenuItem^  saveMenuButton;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsMenuButton;
    private: System::Windows::Forms::ToolStripMenuItem^  exitMenuButton;
    private: System::Windows::Forms::MenuStrip^  mainMenu;
    private: System::Windows::Forms::ToolStripMenuItem^  fileMenu;
    private: System::ComponentModel::IContainer ^  components;

    public :   
        ~MainForm()
        {
        }

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->components = gcnew System::ComponentModel::Container();
            System::ComponentModel::ComponentResourceManager^  resources = gcnew System::ComponentModel::ComponentResourceManager(typeid<MainForm >);
            this->ToolBarImages = gcnew System::Windows::Forms::ImageList(this->components);
            this->mainToolbar = gcnew System::Windows::Forms::ToolStrip();
            this->saveButton = gcnew System::Windows::Forms::ToolStripButton();
            this->takeScreenshot = gcnew System::Windows::Forms::ToolStripButton();
            this->toolStripSeparator1 = gcnew System::Windows::Forms::ToolStripButton();
            this->toolStripButton2 = gcnew System::Windows::Forms::ToolStripButton();
            this->toolStripAssetBrowser = gcnew System::Windows::Forms::ToolStripButton();
            this->leftRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
            this->toolsToolbar = gcnew System::Windows::Forms::ToolStrip();
            this->moveButton = gcnew System::Windows::Forms::ToolStripButton();
            this->rotateButton = gcnew System::Windows::Forms::ToolStripButton();
            this->scaleButton = gcnew System::Windows::Forms::ToolStripButton();
            this->toolStripButton4 = gcnew System::Windows::Forms::ToolStripButton();
            this->orbitViewToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
            this->toolStripSeparator7 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->toolStripComboBox1 = gcnew System::Windows::Forms::ToolStripComboBox();
            this->toolStripSeparator2 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->slowToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
            this->mediumToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
            this->fastToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
            this->rightRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
            this->topRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
            this->mainMenu = gcnew System::Windows::Forms::MenuStrip();
            this->fileMenu = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->newMenuButton = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->toolStripMenuItem1 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->openToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->toolStripMenuItem2 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->saveMenuButton = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->saveAsMenuButton = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->saveSelectedToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->importMeshToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->exportSelectedAsPrefabsToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->exportSelectedToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->mergeToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->toolStripMenuItem3 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->toolStripSeparator5 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->exitMenuButton = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->editToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->undoToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->redoToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->deleteToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->selectObjectsToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->toolStripSeparator12 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->unHideAllToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->unFreezeAllToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->freezeSelectedToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->hideSelectedToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->hideUnselectedToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->viewToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->wireframeToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->toolStripMenuItem4 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->occToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->bboxToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->modifiersToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->flipNormalsToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->invertMeshToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->regenerateTangentsNormalsLightingToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->centerPivotToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->combineSelectedPrefabsToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->splitMeshToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->bakePRTLightingToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->breakMeshintosubelementsToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->buildToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->buildSelectedToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->buildSelectedFastToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->buildAllToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->buildFastToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->gameToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->playToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->reloadSkyToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->editWorldPropertiesToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->helpToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->visitOnlineHelpToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->aboutToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->toolStrip3 = gcnew System::Windows::Forms::ToolStrip();
            this->xToolStripButton = gcnew System::Windows::Forms::ToolStripLabel();
            this->boxX = gcnew System::Windows::Forms::ToolStripTextBox();
            this->toolStripLabel2 = gcnew System::Windows::Forms::ToolStripLabel();
            this->boxY = gcnew System::Windows::Forms::ToolStripTextBox();
            this->toolStripLabel1 = gcnew System::Windows::Forms::ToolStripLabel();
            this->boxZ = gcnew System::Windows::Forms::ToolStripTextBox();
            this->toolStrip2 = gcnew System::Windows::Forms::ToolStrip();
            this->sTGToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
            this->sTGEditBoxToolStripButton = gcnew System::Windows::Forms::ToolStripTextBox();
            this->toolStrip1 = gcnew System::Windows::Forms::ToolStrip();
            this->consoleToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
            this->consoleToolStripTextBox = gcnew System::Windows::Forms::ToolStripTextBox();
            this->toolStripSeparator4 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->toolStripButton1 = gcnew System::Windows::Forms::ToolStripButton();
            this->insertToolBarSep = gcnew System::Windows::Forms::ToolStripSeparator();
            this->changeActor = gcnew System::Windows::Forms::ToolStripButton();
            this->addActorButton = gcnew System::Windows::Forms::ToolStripButton();
            this->actorsCombo = gcnew System::Windows::Forms::ToolStripComboBox();
            this->bottomRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
            this->statusBar = gcnew System::Windows::Forms::StatusStrip();
            this->statusStripPanel1 = gcnew System::Windows::Forms::StatusStripPanel();
            this->toolStripSeparator3 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->toolStripSeparator6 = gcnew System::Windows::Forms::ToolStripSeparator();
            this->importMeshDialog = gcnew System::Windows::Forms::OpenFileDialog();
            this->splitContainer1 = gcnew System::Windows::Forms::SplitContainer();
            this->splitContainer2 = gcnew System::Windows::Forms::SplitContainer();
            this->splitContainer3 = gcnew System::Windows::Forms::SplitContainer();
            this->richTextBox1 = gcnew System::Windows::Forms::RichTextBox();
            this->statisticsBox = gcnew System::Windows::Forms::TextBox();
            this->tabPanelRight = gcnew System::Windows::Forms::TabControl();
            this->tabPage1 = gcnew System::Windows::Forms::TabPage();
            this->propertyGrid1 = gcnew System::Windows::Forms::PropertyGrid();
            this->panel1 = gcnew System::Windows::Forms::Panel();
            this->checkIsExculsion = gcnew System::Windows::Forms::CheckBox();
            this->button4 = gcnew System::Windows::Forms::Button();
            this->button3 = gcnew System::Windows::Forms::Button();
            this->label7 = gcnew System::Windows::Forms::Label();
            this->label6 = gcnew System::Windows::Forms::Label();
            this->listExclusion = gcnew System::Windows::Forms::ListBox();
            this->listSceneMeshes1 = gcnew System::Windows::Forms::ListBox();
            this->panel2 = gcnew System::Windows::Forms::Panel();
            this->label9 = gcnew System::Windows::Forms::Label();
            this->panel3 = gcnew System::Windows::Forms::Panel();
            this->label8 = gcnew System::Windows::Forms::Label();
            this->linkLabel1 = gcnew System::Windows::Forms::LinkLabel();
            this->tabPage2 = gcnew System::Windows::Forms::TabPage();
            this->splitContainer4 = gcnew System::Windows::Forms::SplitContainer();
            this->comboSubmat = gcnew System::Windows::Forms::ComboBox();
            this->labelSubmat = gcnew System::Windows::Forms::Label();
            this->label2 = gcnew System::Windows::Forms::Label();
            this->comboSubmesh = gcnew System::Windows::Forms::ComboBox();
            this->renderProps = gcnew System::Windows::Forms::PropertyGrid();
            this->label5 = gcnew System::Windows::Forms::Label();
            this->label1 = gcnew System::Windows::Forms::Label();
            this->button2 = gcnew System::Windows::Forms::Button();
            this->button1 = gcnew System::Windows::Forms::Button();
            this->listBoxScene = gcnew System::Windows::Forms::ListBox();
            this->listBoxBlockers = gcnew System::Windows::Forms::ListBox();
            this->tabPage3 = gcnew System::Windows::Forms::TabPage();
            this->panel4 = gcnew System::Windows::Forms::Panel();
            this->button7 = gcnew System::Windows::Forms::Button();
            this->clearListButton = gcnew System::Windows::Forms::Button();
            this->button9 = gcnew System::Windows::Forms::Button();
            this->button8 = gcnew System::Windows::Forms::Button();
            this->selectedListBox = gcnew System::Windows::Forms::ListBox();
            this->button6 = gcnew System::Windows::Forms::Button();
            this->button5 = gcnew System::Windows::Forms::Button();
            this->selectedListComboBox = gcnew System::Windows::Forms::ComboBox();
            this->label10 = gcnew System::Windows::Forms::Label();
            this->tabPage4 = gcnew System::Windows::Forms::TabPage();
            this->statsText = gcnew System::Windows::Forms::TextBox();
            this->saveFileDialog = gcnew System::Windows::Forms::SaveFileDialog();
            this->contextMenuStrip1 = gcnew System::Windows::Forms::ContextMenuStrip(this->components);
            this->testToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->hideUnSelectedToolStripMenuItem1 = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->unhideAllToolStripMenuItem1 = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->freezeSelectedToolStripMenuItem1 = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->unFreezeAllToolStripMenuItem1 = gcnew System::Windows::Forms::ToolStripMenuItem();
            this->buttonRemoveBlockers = gcnew System::Windows::Forms::Button();
            this->label4 = gcnew System::Windows::Forms::Label();
            this->label3 = gcnew System::Windows::Forms::Label();
            this->buttonAddBlockers = gcnew System::Windows::Forms::Button();
            this->exportFileDialog = gcnew System::Windows::Forms::SaveFileDialog();
            this->openFileDialog1 = gcnew System::Windows::Forms::OpenFileDialog();
            this->mainToolbar->SuspendLayout();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer))->BeginInit();
            this->toolsToolbar->SuspendLayout();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->rightRaftingContainer))->BeginInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->topRaftingContainer))->BeginInit();
            this->topRaftingContainer->SuspendLayout();
            this->mainMenu->SuspendLayout();
            this->toolStrip3->SuspendLayout();
            this->toolStrip2->SuspendLayout();
            this->toolStrip1->SuspendLayout();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bottomRaftingContainer))->BeginInit();
            this->bottomRaftingContainer->SuspendLayout();
            this->statusBar->SuspendLayout();
            this->splitContainer1->Panel1->SuspendLayout();
            this->splitContainer1->Panel2->SuspendLayout();
            this->splitContainer1->SuspendLayout();
            this->splitContainer2->Panel2->SuspendLayout();
            this->splitContainer2->SuspendLayout();
            this->splitContainer3->Panel1->SuspendLayout();
            this->splitContainer3->Panel2->SuspendLayout();
            this->splitContainer3->SuspendLayout();
            this->tabPanelRight->SuspendLayout();
            this->tabPage1->SuspendLayout();
            this->panel1->SuspendLayout();
            this->panel2->SuspendLayout();
            this->panel3->SuspendLayout();
            this->tabPage2->SuspendLayout();
            this->splitContainer4->Panel1->SuspendLayout();
            this->splitContainer4->Panel2->SuspendLayout();
            this->splitContainer4->SuspendLayout();
            this->tabPage3->SuspendLayout();
            this->panel4->SuspendLayout();
            this->tabPage4->SuspendLayout();
            this->contextMenuStrip1->SuspendLayout();
            this->SuspendLayout();
            this->ToolBarImages->ColorDepth = System::Windows::Forms::ColorDepth::Depth24Bit;
            this->ToolBarImages->ImageSize = System::Drawing::Size(16, 16);
            this->ToolBarImages->ImageStream = (stdcli::language::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"ToolBarImages.ImageStream")));
            this->ToolBarImages->TransparentColor = System::Drawing::Color::Silver;
            this->ToolBarImages->Images->SetKeyName(0, L"");
            this->ToolBarImages->Images->SetKeyName(1, L"");
            this->ToolBarImages->Images->SetKeyName(2, L"");
            this->ToolBarImages->Images->SetKeyName(3, L"");
            this->ToolBarImages->Images->SetKeyName(4, L"");
            this->ToolBarImages->Images->SetKeyName(5, L"");
            this->ToolBarImages->Images->SetKeyName(6, L"UNDO.BMP");
            this->ToolBarImages->Images->SetKeyName(7, L"REDO.BMP");
            this->ToolBarImages->Images->SetKeyName(8, L"move.bmp");
            this->ToolBarImages->Images->SetKeyName(9, L"rotate.bmp");
            this->ToolBarImages->Images->SetKeyName(10, L"scale.bmp");
            this->mainToolbar->ImageList = this->ToolBarImages;
            this->mainToolbar->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(5) {this->saveButton, this->takeScreenshot, this->toolStripSeparator1, this->toolStripButton2, this->toolStripAssetBrowser});
            this->mainToolbar->Location = System::Drawing::Point(309, 21);
            this->mainToolbar->Name = L"mainToolbar";
            this->mainToolbar->Raft = System::Windows::Forms::RaftingSides::Top;
            this->mainToolbar->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
            this->mainToolbar->SaveSettings = true;
            this->mainToolbar->SettingsKey = L"MainForm.mainToolbar";
            this->mainToolbar->Size = System::Drawing::Size(100, 26);
            this->mainToolbar->TabIndex = 0;
            this->saveButton->AutoSize = false;
            this->saveButton->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"saveButton.Image")));
            this->saveButton->Name = L"saveButton";
            this->saveButton->SettingsKey = L"MainForm.toolStripButton1";
            this->saveButton->Size = System::Drawing::Size(23, 23);
            this->saveButton->ToolTipText = L"Save";
            this->saveButton->Click += gcnew System::EventHandler(this, &MainForm::saveButton_Click);
            this->takeScreenshot->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"takeScreenshot.Image")));
            this->takeScreenshot->Name = L"takeScreenshot";
            this->takeScreenshot->SettingsKey = L"MainForm.toolStripButton2";
            this->takeScreenshot->ToolTipText = L"Take Screenshot!";
            this->takeScreenshot->Click += gcnew System::EventHandler(this, &MainForm::takeScreenshot_Click);
            this->toolStripSeparator1->Name = L"toolStripSeparator1";
            this->toolStripSeparator1->SettingsKey = L"MainForm.toolStripButton1";
            this->toolStripButton2->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton2.Image")));
            this->toolStripButton2->Name = L"toolStripButton2";
            this->toolStripButton2->SettingsKey = L"MainForm.toolStripButton2";
            this->toolStripButton2->ToolTipText = L"Select Object(s)...";
            this->toolStripButton2->Click += gcnew System::EventHandler(this, &MainForm::toolStripButton2_Click);
            this->toolStripAssetBrowser->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripAssetBrowser.Image")));
            this->toolStripAssetBrowser->Name = L"toolStripAssetBrowser";
            this->toolStripAssetBrowser->SettingsKey = L"MainForm.toolStripButton5";
            this->toolStripAssetBrowser->ToolTipText = L"Asset Browser";
            this->toolStripAssetBrowser->Click += gcnew System::EventHandler(this, &MainForm::toolStripAssetBrowser_Click);
            this->leftRaftingContainer->Dock = System::Windows::Forms::DockStyle::Left;
            this->leftRaftingContainer->Name = L"leftRaftingContainer";
            this->toolsToolbar->ImageList = this->ToolBarImages;
            this->toolsToolbar->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(11) {this->moveButton, this->rotateButton, this->scaleButton, this->toolStripButton4, this->orbitViewToolStripButton, this->toolStripSeparator7, this->toolStripComboBox1, this->toolStripSeparator2, this->slowToolStripButton, this->mediumToolStripButton, this->fastToolStripButton});
            this->toolsToolbar->Location = System::Drawing::Point(0, 47);
            this->toolsToolbar->Name = L"toolsToolbar";
            this->toolsToolbar->Raft = System::Windows::Forms::RaftingSides::Top;
            this->toolsToolbar->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
            this->toolsToolbar->SaveSettings = true;
            this->toolsToolbar->SettingsKey = L"MainForm.toolsToolbar";
            this->toolsToolbar->TabIndex = 2;
            this->toolsToolbar->Click += gcnew System::EventHandler(this, &MainForm::toolsToolbar_Click);
            this->moveButton->AutoSize = false;
            this->moveButton->Checked = true;
            this->moveButton->CheckState = System::Windows::Forms::CheckState::Checked;
            this->moveButton->ImageIndex = 8;
            this->moveButton->Name = L"moveButton";
            this->moveButton->SettingsKey = L"MainForm.toolStripButton1";
            this->moveButton->Size = System::Drawing::Size(23, 23);
            this->moveButton->ToolTipText = L"Move";
            this->moveButton->Click += gcnew System::EventHandler(this, &MainForm::moveButton_Click);
            this->rotateButton->AutoSize = false;
            this->rotateButton->ImageIndex = 9;
            this->rotateButton->Name = L"rotateButton";
            this->rotateButton->SettingsKey = L"MainForm.toolStripButton1";
            this->rotateButton->Size = System::Drawing::Size(23, 23);
            this->rotateButton->ToolTipText = L"Rotate";
            this->rotateButton->Click += gcnew System::EventHandler(this, &MainForm::rotateButton_Click);
            this->scaleButton->AutoSize = false;
            this->scaleButton->ImageIndex = 10;
            this->scaleButton->Name = L"scaleButton";
            this->scaleButton->SettingsKey = L"MainForm.toolStripButton1";
            this->scaleButton->Size = System::Drawing::Size(23, 23);
            this->scaleButton->ToolTipText = L"Scale";
            this->scaleButton->Click += gcnew System::EventHandler(this, &MainForm::scaleButton_Click);
            this->toolStripButton4->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton4.Image")));
            this->toolStripButton4->Name = L"toolStripButton4";
            this->toolStripButton4->SettingsKey = L"MainForm.toolStripButton4";
            this->toolStripButton4->ToolTipText = L"Snap View To Selected";
            this->toolStripButton4->Click += gcnew System::EventHandler(this, &MainForm::toolStripButton4_Click_1);
            this->orbitViewToolStripButton->CheckOnClick = true;
            this->orbitViewToolStripButton->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"orbitViewToolStripButton.Image")));
            this->orbitViewToolStripButton->Name = L"orbitViewToolStripButton";
            this->orbitViewToolStripButton->SettingsKey = L"MainForm.orbitViewToolStripButton";
            this->orbitViewToolStripButton->Click += gcnew System::EventHandler(this, &MainForm::orbitViewToolStripButton_Click);
            this->toolStripSeparator7->Name = L"toolStripSeparator7";
            this->toolStripSeparator7->SettingsKey = L"MainForm.toolStripSeparator7";
            this->toolStripComboBox1->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->toolStripComboBox1->Items->AddRange(gcnew stdcli::language::array<System::Object^ >(7) {L"Perspective", L"Left", L"Top", L"Right", L"Bottom", L"Front", L"Back"});
            this->toolStripComboBox1->Name = L"toolStripComboBox1";
            this->toolStripComboBox1->SettingsKey = L"MainForm.toolStripComboBox1";
            this->toolStripComboBox1->Size = System::Drawing::Size(92, 25);
            this->toolStripComboBox1->Text = L"Perspective";
            this->toolStripComboBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::toolStripComboBox1_SelectedIndexChanged);
            this->toolStripComboBox1->Click += gcnew System::EventHandler(this, &MainForm::toolStripComboBox1_Click);
            this->toolStripSeparator2->Name = L"toolStripSeparator2";
            this->toolStripSeparator2->SettingsKey = L"MainForm.toolStripSeparator2";
            this->slowToolStripButton->Name = L"slowToolStripButton";
            this->slowToolStripButton->SettingsKey = L"MainForm.slowToolStripButton";
            this->slowToolStripButton->Text = L"Slow";
            this->slowToolStripButton->Click += gcnew System::EventHandler(this, &MainForm::slowToolStripButton_Click);
            this->mediumToolStripButton->Checked = true;
            this->mediumToolStripButton->CheckOnClick = true;
            this->mediumToolStripButton->CheckState = System::Windows::Forms::CheckState::Checked;
            this->mediumToolStripButton->Name = L"mediumToolStripButton";
            this->mediumToolStripButton->SettingsKey = L"MainForm.mediumToolStripButton";
            this->mediumToolStripButton->Text = L"Medium";
            this->mediumToolStripButton->Click += gcnew System::EventHandler(this, &MainForm::mediumToolStripButton_Click);
            this->fastToolStripButton->Name = L"fastToolStripButton";
            this->fastToolStripButton->SettingsKey = L"MainForm.fastToolStripButton";
            this->fastToolStripButton->Text = L"Fast";
            this->fastToolStripButton->Click += gcnew System::EventHandler(this, &MainForm::fastToolStripButton_Click);
            this->rightRaftingContainer->Dock = System::Windows::Forms::DockStyle::Right;
            this->rightRaftingContainer->Name = L"rightRaftingContainer";
            this->topRaftingContainer->Controls->Add(this->mainMenu);
            this->topRaftingContainer->Controls->Add(this->toolStrip3);
            this->topRaftingContainer->Controls->Add(this->mainToolbar);
            this->topRaftingContainer->Controls->Add(this->toolStrip2);
            this->topRaftingContainer->Controls->Add(this->toolsToolbar);
            this->topRaftingContainer->Controls->Add(this->toolStrip1);
            this->topRaftingContainer->Dock = System::Windows::Forms::DockStyle::Top;
            this->topRaftingContainer->Name = L"topRaftingContainer";
            this->topRaftingContainer->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
            this->mainMenu->ImageList = this->ToolBarImages;
            this->mainMenu->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(7) {this->fileMenu, this->editToolStripMenuItem, this->viewToolStripMenuItem, this->modifiersToolStripMenuItem, this->buildToolStripMenuItem, this->gameToolStripMenuItem, this->helpToolStripMenuItem});
            this->mainMenu->Location = System::Drawing::Point(0, 0);
            this->mainMenu->Name = L"mainMenu";
            this->mainMenu->Padding = System::Windows::Forms::Padding(6, 2, 0, 2);
            this->mainMenu->Raft = System::Windows::Forms::RaftingSides::Top;
            this->mainMenu->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
            this->mainMenu->ShowItemToolTips = true;
            this->mainMenu->TabIndex = 3;
            this->mainMenu->Text = L"menuStrip1";
            this->mainMenu->Click += gcnew System::EventHandler(this, &MainForm::mainMenu_Click);
            this->fileMenu->DropDownItems->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(14) {this->newMenuButton, this->toolStripMenuItem1, this->openToolStripMenuItem, this->toolStripMenuItem2, this->saveMenuButton, this->saveAsMenuButton, this->saveSelectedToolStripMenuItem, this->importMeshToolStripMenuItem, this->exportSelectedAsPrefabsToolStripMenuItem, this->exportSelectedToolStripMenuItem, this->mergeToolStripMenuItem, this->toolStripMenuItem3, this->toolStripSeparator5, this->exitMenuButton});
            this->fileMenu->Name = L"fileMenu";
            this->fileMenu->SettingsKey = L"MainForm.fileToolStripMenuItem";
            this->fileMenu->Text = L"File";
            this->fileMenu->Click += gcnew System::EventHandler(this, &MainForm::fileToolStripMenuItem_Click);
            this->newMenuButton->Name = L"newMenuButton";
            this->newMenuButton->SettingsKey = L"MainForm.newMenuButton";
            this->newMenuButton->Text = L"New";
            this->newMenuButton->Click += gcnew System::EventHandler(this, &MainForm::newMenuButton_Click);
            this->toolStripMenuItem1->Name = L"toolStripMenuItem1";
            this->toolStripMenuItem1->SettingsKey = L"MainForm.toolStripMenuItem1";
            this->openToolStripMenuItem->Name = L"openToolStripMenuItem";
            this->openToolStripMenuItem->SettingsKey = L"MainForm.openToolStripMenuItem";
            this->openToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::O));
            this->openToolStripMenuItem->Text = L"Open...";
            this->openToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::openToolStripMenuItem_Click);
            this->toolStripMenuItem2->Name = L"toolStripMenuItem2";
            this->toolStripMenuItem2->SettingsKey = L"MainForm.toolStripMenuItem2";
            this->saveMenuButton->Name = L"saveMenuButton";
            this->saveMenuButton->SettingsKey = L"MainForm.saveToolStripMenuItem";
            this->saveMenuButton->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::S));
            this->saveMenuButton->Text = L"Save";
            this->saveMenuButton->Click += gcnew System::EventHandler(this, &MainForm::saveMenuButton_Click);
            this->saveAsMenuButton->Name = L"saveAsMenuButton";
            this->saveAsMenuButton->SettingsKey = L"MainForm.saveAsToolStripMenuItem";
            this->saveAsMenuButton->Text = L"Save As...";
            this->saveAsMenuButton->Click += gcnew System::EventHandler(this, &MainForm::saveAsMenuButton_Click);
            this->saveSelectedToolStripMenuItem->Name = L"saveSelectedToolStripMenuItem";
            this->saveSelectedToolStripMenuItem->SettingsKey = L"MainForm.saveSelectedToolStripMenuItem";
            this->saveSelectedToolStripMenuItem->Text = L"Save Selected...";
            this->saveSelectedToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveSelectedToolStripMenuItem_Click);
            this->importMeshToolStripMenuItem->Name = L"importMeshToolStripMenuItem";
            this->importMeshToolStripMenuItem->SettingsKey = L"MainForm.importMeshToolStripMenuItem";
            this->importMeshToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::I));
            this->importMeshToolStripMenuItem->Text = L"Import Mesh...";
            this->importMeshToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::importMeshToolStripMenuItem_Click);
            this->exportSelectedAsPrefabsToolStripMenuItem->Name = L"exportSelectedAsPrefabsToolStripMenuItem";
            this->exportSelectedAsPrefabsToolStripMenuItem->SettingsKey = L"MainForm.exportSelectedAsPrefabsToolStripMenuItem";
            this->exportSelectedAsPrefabsToolStripMenuItem->Text = L"Export Selected As Prefabs...";
            this->exportSelectedAsPrefabsToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::exportSelectedAsPrefabsToolStripMenuItem_Click);
            this->exportSelectedToolStripMenuItem->Name = L"exportSelectedToolStripMenuItem";
            this->exportSelectedToolStripMenuItem->SettingsKey = L"MainForm.exportSelectedToolStripMenuItem";
            this->exportSelectedToolStripMenuItem->Text = L"Export Selected...";
            this->exportSelectedToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::exportSelectedToolStripMenuItem_Click);
            this->mergeToolStripMenuItem->Name = L"mergeToolStripMenuItem";
            this->mergeToolStripMenuItem->SettingsKey = L"MainForm.mergeToolStripMenuItem";
            this->mergeToolStripMenuItem->Text = L"Merge...";
            this->mergeToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::mergeToolStripMenuItem_Click);
            this->toolStripMenuItem3->Name = L"toolStripMenuItem3";
            this->toolStripMenuItem3->SettingsKey = L"MainForm.toolStripMenuItem3";
            this->toolStripSeparator5->Name = L"toolStripSeparator5";
            this->toolStripSeparator5->SettingsKey = L"MainForm.toolStripMenuItem3";
            this->exitMenuButton->Name = L"exitMenuButton";
            this->exitMenuButton->SettingsKey = L"MainForm.exitToolStripMenuItem";
            this->exitMenuButton->Text = L"Exit";
            this->exitMenuButton->Click += gcnew System::EventHandler(this, &MainForm::exitMenuButton_Click_1);
            this->editToolStripMenuItem->DropDownItems->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(10) {this->undoToolStripMenuItem, this->redoToolStripMenuItem, this->deleteToolStripMenuItem, this->selectObjectsToolStripMenuItem, this->toolStripSeparator12, this->unHideAllToolStripMenuItem, this->unFreezeAllToolStripMenuItem, this->freezeSelectedToolStripMenuItem, this->hideSelectedToolStripMenuItem, this->hideUnselectedToolStripMenuItem});
            this->editToolStripMenuItem->Name = L"editToolStripMenuItem";
            this->editToolStripMenuItem->SettingsKey = L"MainForm.editToolStripMenuItem";
            this->editToolStripMenuItem->Text = L"Edit";
            this->editToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::editToolStripMenuItem_Click);
            this->undoToolStripMenuItem->Name = L"undoToolStripMenuItem";
            this->undoToolStripMenuItem->SettingsKey = L"MainForm.undoToolStripMenuItem";
            this->undoToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::Z));
            this->undoToolStripMenuItem->Text = L"Undo";
            this->undoToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::undoToolStripMenuItem_Click);
            this->redoToolStripMenuItem->Name = L"redoToolStripMenuItem";
            this->redoToolStripMenuItem->SettingsKey = L"MainForm.redoToolStripMenuItem";
            this->redoToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::Y));
            this->redoToolStripMenuItem->Text = L"Redo";
            this->redoToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::redoToolStripMenuItem_Click_1);
            this->deleteToolStripMenuItem->Name = L"deleteToolStripMenuItem";
            this->deleteToolStripMenuItem->SettingsKey = L"MainForm.deleteToolStripMenuItem";
            this->deleteToolStripMenuItem->ShortcutKeys = System::Windows::Forms::Keys::Delete;
            this->deleteToolStripMenuItem->Text = L"Delete";
            this->deleteToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::deleteToolStripMenuItem_Click_1);
            this->selectObjectsToolStripMenuItem->Name = L"selectObjectsToolStripMenuItem";
            this->selectObjectsToolStripMenuItem->SettingsKey = L"MainForm.selectObjectsToolStripMenuItem";
            this->selectObjectsToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Alt | System::Windows::Forms::Keys::S));
            this->selectObjectsToolStripMenuItem->Text = L"Select Object(s)...";
            this->selectObjectsToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::selectObjectsToolStripMenuItem_Click);
            this->toolStripSeparator12->Name = L"toolStripSeparator12";
            this->toolStripSeparator12->SettingsKey = L"MainForm.toolStripMenuItem1";
            this->unHideAllToolStripMenuItem->Name = L"unHideAllToolStripMenuItem";
            this->unHideAllToolStripMenuItem->SettingsKey = L"MainForm.unHideAllToolStripMenuItem";
            this->unHideAllToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::U));
            this->unHideAllToolStripMenuItem->Text = L"UnHide All";
            this->unHideAllToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::unHideAllToolStripMenuItem_Click);
            this->unFreezeAllToolStripMenuItem->Name = L"unFreezeAllToolStripMenuItem";
            this->unFreezeAllToolStripMenuItem->SettingsKey = L"MainForm.unFreezeAllToolStripMenuItem";
            this->unFreezeAllToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::G));
            this->unFreezeAllToolStripMenuItem->Text = L"UnFreeze All";
            this->unFreezeAllToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::unFreezeAllToolStripMenuItem_Click);
            this->freezeSelectedToolStripMenuItem->Name = L"freezeSelectedToolStripMenuItem";
            this->freezeSelectedToolStripMenuItem->SettingsKey = L"MainForm.freezeSelectedToolStripMenuItem";
            this->freezeSelectedToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::F));
            this->freezeSelectedToolStripMenuItem->Text = L"Freeze Selected";
            this->freezeSelectedToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::freezeSelectedToolStripMenuItem_Click);
            this->hideSelectedToolStripMenuItem->Name = L"hideSelectedToolStripMenuItem";
            this->hideSelectedToolStripMenuItem->SettingsKey = L"MainForm.hideSelectedToolStripMenuItem";
            this->hideSelectedToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::H));
            this->hideSelectedToolStripMenuItem->Text = L"Hide Selected";
            this->hideSelectedToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::hideSelectedToolStripMenuItem_Click);
            this->hideUnselectedToolStripMenuItem->Name = L"hideUnselectedToolStripMenuItem";
            this->hideUnselectedToolStripMenuItem->SettingsKey = L"MainForm.hideUnselectedToolStripMenuItem";
            this->hideUnselectedToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::R));
            this->hideUnselectedToolStripMenuItem->Text = L"Hide Unselected";
            this->hideUnselectedToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::hideUnselectedToolStripMenuItem_Click);
            this->viewToolStripMenuItem->DropDownItems->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(4) {this->wireframeToolStripMenuItem, this->toolStripMenuItem4, this->occToolStripMenuItem, this->bboxToolStripMenuItem});
            this->viewToolStripMenuItem->Name = L"viewToolStripMenuItem";
            this->viewToolStripMenuItem->SettingsKey = L"MainForm.viewToolStripMenuItem";
            this->viewToolStripMenuItem->Text = L"View";
            this->viewToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::viewToolStripMenuItem_Click);
            this->wireframeToolStripMenuItem->CheckOnClick = true;
            this->wireframeToolStripMenuItem->Name = L"wireframeToolStripMenuItem";
            this->wireframeToolStripMenuItem->SettingsKey = L"MainForm.wireframeToolStripMenuItem";
            this->wireframeToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::W));
            this->wireframeToolStripMenuItem->Text = L"Wireframe";
            this->wireframeToolStripMenuItem->ToolTipText = L"Wireframe rendering mode toggle";
            this->wireframeToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::wireframeToolStripMenuItem_Click);
            this->toolStripMenuItem4->Name = L"toolStripMenuItem4";
            this->toolStripMenuItem4->SettingsKey = L"MainForm.toolStripMenuItem4";
            this->occToolStripMenuItem->Checked = true;
            this->occToolStripMenuItem->CheckOnClick = true;
            this->occToolStripMenuItem->CheckState = System::Windows::Forms::CheckState::Checked;
            this->occToolStripMenuItem->Name = L"occToolStripMenuItem";
            this->occToolStripMenuItem->SettingsKey = L"MainForm.occToolStripMenuItem";
            this->occToolStripMenuItem->Text = L"Occlusion Culling";
            this->occToolStripMenuItem->ToolTipText = L"Toggle Occlusion culling";
            this->occToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::occToolStripMenuItem_Click);
            this->bboxToolStripMenuItem->CheckOnClick = true;
            this->bboxToolStripMenuItem->Name = L"bboxToolStripMenuItem";
            this->bboxToolStripMenuItem->SettingsKey = L"MainForm.bboxToolStripMenuItem";
            this->bboxToolStripMenuItem->Text = L"Draw Occlusion Tree";
            this->bboxToolStripMenuItem->ToolTipText = L"Toggle Occlusion Tree Bounding boxes rendering";
            this->bboxToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::bboxToolStripMenuItem_Click);
            this->modifiersToolStripMenuItem->DropDownItems->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(8) {this->flipNormalsToolStripMenuItem, this->invertMeshToolStripMenuItem, this->regenerateTangentsNormalsLightingToolStripMenuItem, this->centerPivotToolStripMenuItem, this->combineSelectedPrefabsToolStripMenuItem, this->splitMeshToolStripMenuItem, this->bakePRTLightingToolStripMenuItem, this->breakMeshintosubelementsToolStripMenuItem});
            this->modifiersToolStripMenuItem->Name = L"modifiersToolStripMenuItem";
            this->modifiersToolStripMenuItem->SettingsKey = L"MainForm.modifiersToolStripMenuItem";
            this->modifiersToolStripMenuItem->Text = L"Modifiers";
            this->modifiersToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::modifiersToolStripMenuItem_Click);
            this->flipNormalsToolStripMenuItem->Name = L"flipNormalsToolStripMenuItem";
            this->flipNormalsToolStripMenuItem->SettingsKey = L"MainForm.flipNormalsToolStripMenuItem";
            this->flipNormalsToolStripMenuItem->Text = L"Flip Normals";
            this->flipNormalsToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::flipNormalsToolStripMenuItem_Click);
            this->invertMeshToolStripMenuItem->Name = L"invertMeshToolStripMenuItem";
            this->invertMeshToolStripMenuItem->SettingsKey = L"MainForm.invertMeshToolStripMenuItem";
            this->invertMeshToolStripMenuItem->Text = L"Invert Mesh";
            this->invertMeshToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::invertMeshToolStripMenuItem_Click);
            this->regenerateTangentsNormalsLightingToolStripMenuItem->Name = L"regenerateTangentsNormalsLightingToolStripMenuItem";
            this->regenerateTangentsNormalsLightingToolStripMenuItem->SettingsKey = L"MainForm.regenerateTangentsNormalsLightingToolStripMenuItem";
            this->regenerateTangentsNormalsLightingToolStripMenuItem->Text = L"Regenerate Tangents && Normals (Lighting)";
            this->regenerateTangentsNormalsLightingToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::regenerateTangentsNormalsLightingToolStripMenuItem_Click);
            this->centerPivotToolStripMenuItem->Name = L"centerPivotToolStripMenuItem";
            this->centerPivotToolStripMenuItem->SettingsKey = L"MainForm.centerPivotToolStripMenuItem";
            this->centerPivotToolStripMenuItem->Text = L"Center Pivot";
            this->centerPivotToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::centerPivotToolStripMenuItem_Click);
            this->combineSelectedPrefabsToolStripMenuItem->Name = L"combineSelectedPrefabsToolStripMenuItem";
            this->combineSelectedPrefabsToolStripMenuItem->SettingsKey = L"MainForm.combineSelectedPrefabsToolStripMenuItem";
            this->combineSelectedPrefabsToolStripMenuItem->Text = L"Combine Selected Prefabs";
            this->combineSelectedPrefabsToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::combineSelectedPrefabsToolStripMenuItem_Click);
            this->splitMeshToolStripMenuItem->Name = L"splitMeshToolStripMenuItem";
            this->splitMeshToolStripMenuItem->SettingsKey = L"MainForm.splitMeshToolStripMenuItem";
            this->splitMeshToolStripMenuItem->Text = L"Split Mesh...";
            this->splitMeshToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::splitMeshToolStripMenuItem_Click);
            this->bakePRTLightingToolStripMenuItem->Name = L"bakePRTLightingToolStripMenuItem";
            this->bakePRTLightingToolStripMenuItem->SettingsKey = L"MainForm.bakePRTLightingToolStripMenuItem";
            this->bakePRTLightingToolStripMenuItem->Text = L"Bake PRT Lighting";
            this->bakePRTLightingToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::bakePRTLightingToolStripMenuItem_Click);
            this->breakMeshintosubelementsToolStripMenuItem->Name = L"breakMeshintosubelementsToolStripMenuItem";
            this->breakMeshintosubelementsToolStripMenuItem->SettingsKey = L"MainForm.breakMeshintosubelementsToolStripMenuItem";
            this->breakMeshintosubelementsToolStripMenuItem->Text = L"Break Mesh into subelements";
            this->breakMeshintosubelementsToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::breakMeshintosubelementsToolStripMenuItem_Click);
            this->buildToolStripMenuItem->DropDownItems->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(4) {this->buildSelectedToolStripMenuItem, this->buildSelectedFastToolStripMenuItem, this->buildAllToolStripMenuItem, this->buildFastToolStripMenuItem});
            this->buildToolStripMenuItem->Name = L"buildToolStripMenuItem";
            this->buildToolStripMenuItem->SettingsKey = L"MainForm.buildToolStripMenuItem";
            this->buildToolStripMenuItem->Text = L"Build";
            this->buildToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::buildToolStripMenuItem_Click);
            this->buildSelectedToolStripMenuItem->Name = L"buildSelectedToolStripMenuItem";
            this->buildSelectedToolStripMenuItem->SettingsKey = L"MainForm.buildSelectedToolStripMenuItem";
            this->buildSelectedToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Alt | System::Windows::Forms::Keys::S));
            this->buildSelectedToolStripMenuItem->Text = L"Build Selected";
            this->buildSelectedToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::buildSelectedToolStripMenuItem_Click);
            this->buildSelectedFastToolStripMenuItem->Name = L"buildSelectedFastToolStripMenuItem";
            this->buildSelectedFastToolStripMenuItem->SettingsKey = L"MainForm.buildSelectedFastToolStripMenuItem";
            this->buildSelectedFastToolStripMenuItem->Text = L"Build Selected (Fast)";
            this->buildSelectedFastToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::buildSelectedFastToolStripMenuItem_Click);
            this->buildAllToolStripMenuItem->Name = L"buildAllToolStripMenuItem";
            this->buildAllToolStripMenuItem->SettingsKey = L"MainForm.buildAllToolStripMenuItem";
            this->buildAllToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Alt | System::Windows::Forms::Keys::A));
            this->buildAllToolStripMenuItem->Text = L"Build All";
            this->buildAllToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::buildAllToolStripMenuItem_Click);
            this->buildFastToolStripMenuItem->Name = L"buildFastToolStripMenuItem";
            this->buildFastToolStripMenuItem->SettingsKey = L"MainForm.buildFastToolStripMenuItem";
            this->buildFastToolStripMenuItem->Text = L"Build All (Fast)";
            this->buildFastToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::buildFastToolStripMenuItem_Click);
            this->gameToolStripMenuItem->DropDownItems->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(3) {this->playToolStripMenuItem, this->reloadSkyToolStripMenuItem, this->editWorldPropertiesToolStripMenuItem});
            this->gameToolStripMenuItem->Name = L"gameToolStripMenuItem";
            this->gameToolStripMenuItem->SettingsKey = L"MainForm.gameToolStripMenuItem";
            this->gameToolStripMenuItem->Text = L"Game";
            this->gameToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::gameToolStripMenuItem_Click);
            this->playToolStripMenuItem->Name = L"playToolStripMenuItem";
            this->playToolStripMenuItem->SettingsKey = L"MainForm.playToolStripMenuItem";
            this->playToolStripMenuItem->ShortcutKeys = System::Windows::Forms::Keys::F5;
            this->playToolStripMenuItem->Text = L"Play!";
            this->playToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::playToolStripMenuItem_Click);
            this->reloadSkyToolStripMenuItem->Name = L"reloadSkyToolStripMenuItem";
            this->reloadSkyToolStripMenuItem->SettingsKey = L"MainForm.reloadSkyToolStripMenuItem";
            this->reloadSkyToolStripMenuItem->Text = L"Reload Sky";
            this->reloadSkyToolStripMenuItem->ToolTipText = L"Reloads the sky script and configuration";
            this->reloadSkyToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::reloadSkyToolStripMenuItem_Click);
            this->editWorldPropertiesToolStripMenuItem->Name = L"editWorldPropertiesToolStripMenuItem";
            this->editWorldPropertiesToolStripMenuItem->SettingsKey = L"MainForm.editWorldPropertiesToolStripMenuItem";
            this->editWorldPropertiesToolStripMenuItem->Text = L"Edit World Properties";
            this->editWorldPropertiesToolStripMenuItem->ToolTipText = L"Displays world properties in General tab on sidebar for editing";
            this->editWorldPropertiesToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::editWorldPropertiesToolStripMenuItem_Click);
            this->helpToolStripMenuItem->DropDownItems->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(2) {this->visitOnlineHelpToolStripMenuItem, this->aboutToolStripMenuItem});
            this->helpToolStripMenuItem->Name = L"helpToolStripMenuItem";
            this->helpToolStripMenuItem->SaveSettings = true;
            this->helpToolStripMenuItem->SettingsKey = L"MainForm.helpToolStripMenuItem";
            this->helpToolStripMenuItem->Text = L"Help";
            this->helpToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::helpToolStripMenuItem_Click);
            this->visitOnlineHelpToolStripMenuItem->Name = L"visitOnlineHelpToolStripMenuItem";
            this->visitOnlineHelpToolStripMenuItem->SettingsKey = L"MainForm.visitOnlineHelpToolStripMenuItem";
            this->visitOnlineHelpToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::F1));
            this->visitOnlineHelpToolStripMenuItem->Text = L"Visit Online Help...";
            this->visitOnlineHelpToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::visitOnlineHelpToolStripMenuItem_Click);
            this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
            this->aboutToolStripMenuItem->SettingsKey = L"MainForm.aboutToolStripMenuItem";
            this->aboutToolStripMenuItem->Text = L"About...";
            this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::aboutToolStripMenuItem_Click);
            this->toolStrip3->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(6) {this->xToolStripButton, this->boxX, this->toolStripLabel2, this->boxY, this->toolStripLabel1, this->boxZ});
            this->toolStrip3->Location = System::Drawing::Point(0, 21);
            this->toolStrip3->Name = L"toolStrip3";
            this->toolStrip3->Raft = System::Windows::Forms::RaftingSides::Top;
            this->toolStrip3->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
            this->toolStrip3->SaveSettings = true;
            this->toolStrip3->SettingsKey = L"MainForm.toolStrip3";
            this->toolStrip3->TabIndex = 5;
            this->toolStrip3->Text = L"toolStrip3";
            this->toolStrip3->Layout += gcnew System::Windows::Forms::LayoutEventHandler(this, &MainForm::toolStrip3_Layout);
            this->toolStrip3->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::toolStrip3_MouseMove);
            this->xToolStripButton->Name = L"xToolStripButton";
            this->xToolStripButton->SettingsKey = L"MainForm.xToolStripButton";
            this->xToolStripButton->Text = L"X";
            this->xToolStripButton->Click += gcnew System::EventHandler(this, &MainForm::xToolStripButton_Click);
            this->boxX->MaxLength = 20;
            this->boxX->Name = L"boxX";
            this->boxX->SettingsKey = L"MainForm.toolStripTextBox1";
            this->boxX->Size = System::Drawing::Size(50, 25);
            this->boxX->Text = L"0";
            this->boxX->Click += gcnew System::EventHandler(this, &MainForm::boxX_Click);
            this->toolStripLabel2->Name = L"toolStripLabel2";
            this->toolStripLabel2->SettingsKey = L"MainForm.xToolStripButton";
            this->toolStripLabel2->Text = L"Y";
            this->toolStripLabel2->Click += gcnew System::EventHandler(this, &MainForm::xToolStripButton_Click);
            this->boxY->MaxLength = 20;
            this->boxY->Name = L"boxY";
            this->boxY->SettingsKey = L"MainForm.toolStripTextBox1";
            this->boxY->Size = System::Drawing::Size(50, 25);
            this->boxY->Text = L"0";
            this->boxY->Click += gcnew System::EventHandler(this, &MainForm::boxY_Click);
            this->toolStripLabel1->Name = L"toolStripLabel1";
            this->toolStripLabel1->SettingsKey = L"MainForm.xToolStripButton";
            this->toolStripLabel1->Text = L"Z";
            this->toolStripLabel1->Click += gcnew System::EventHandler(this, &MainForm::xToolStripButton_Click);
            this->boxZ->MaxLength = 20;
            this->boxZ->Name = L"boxZ";
            this->boxZ->SettingsKey = L"MainForm.toolStripTextBox1";
            this->boxZ->Size = System::Drawing::Size(50, 25);
            this->boxZ->Text = L"0";
            this->boxZ->Click += gcnew System::EventHandler(this, &MainForm::toolStripTextBox3_Click);
            this->toolStrip2->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(2) {this->sTGToolStripButton, this->sTGEditBoxToolStripButton});
            this->toolStrip2->Location = System::Drawing::Point(206, 21);
            this->toolStrip2->Name = L"toolStrip2";
            this->toolStrip2->Raft = System::Windows::Forms::RaftingSides::Top;
            this->toolStrip2->SaveSettings = true;
            this->toolStrip2->SettingsKey = L"MainForm.toolStrip2";
            this->toolStrip2->TabIndex = 6;
            this->toolStrip2->Text = L"toolStrip2";
            this->sTGToolStripButton->CheckOnClick = true;
            this->sTGToolStripButton->Name = L"sTGToolStripButton";
            this->sTGToolStripButton->SettingsKey = L"MainForm.sTGToolStripButton";
            this->sTGToolStripButton->Text = L"Snap:";
            this->sTGToolStripButton->ToolTipText = L"Snap To Grid";
            this->sTGToolStripButton->Click += gcnew System::EventHandler(this, &MainForm::sTGToolStripButton_Click);
            this->sTGEditBoxToolStripButton->Name = L"sTGEditBoxToolStripButton";
            this->sTGEditBoxToolStripButton->SaveSettings = true;
            this->sTGEditBoxToolStripButton->SettingsKey = L"MainForm.sTGEditBoxToolStripButton";
            this->sTGEditBoxToolStripButton->Size = System::Drawing::Size(35, 25);
            this->sTGEditBoxToolStripButton->Text = L"1";
            this->sTGEditBoxToolStripButton->Click += gcnew System::EventHandler(this, &MainForm::sTGEditBoxToolStripButton_Click);
            this->toolStrip1->Dock = System::Windows::Forms::DockStyle::None;
            this->toolStrip1->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(8) {this->consoleToolStripButton, this->consoleToolStripTextBox, this->toolStripSeparator4, this->toolStripButton1, this->insertToolBarSep, this->changeActor, this->addActorButton, this->actorsCombo});
            this->toolStrip1->Location = System::Drawing::Point(361, 47);
            this->toolStrip1->Name = L"toolStrip1";
            this->toolStrip1->Raft = System::Windows::Forms::RaftingSides::Top;
            this->toolStrip1->RenderMode = System::Windows::Forms::ToolStripRenderMode::Professional;
            this->toolStrip1->SaveSettings = true;
            this->toolStrip1->SettingsKey = L"MainForm.toolStrip1";
            this->toolStrip1->Size = System::Drawing::Size(399, 26);
            this->toolStrip1->TabIndex = 4;
            this->toolStrip1->Text = L"toolStrip1";
            this->consoleToolStripButton->Name = L"consoleToolStripButton";
            this->consoleToolStripButton->SettingsKey = L"MainForm.consoleToolStripButton";
            this->consoleToolStripButton->Text = L"Console:";
            this->consoleToolStripTextBox->AcceptsReturn = true;
            this->consoleToolStripTextBox->Name = L"consoleToolStripTextBox";
            this->consoleToolStripTextBox->SaveSettings = true;
            this->consoleToolStripTextBox->SettingsKey = L"MainForm.consoleToolStripTextBox";
            this->consoleToolStripTextBox->Size = System::Drawing::Size(111, 26);
            this->toolStripSeparator4->Name = L"toolStripSeparator4";
            this->toolStripSeparator4->SettingsKey = L"MainForm.toolStripSeparator2";
            this->toolStripSeparator4->Click += gcnew System::EventHandler(this, &MainForm::toolStripSeparator3_Click);
            this->toolStripButton1->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton1.Image")));
            this->toolStripButton1->Name = L"toolStripButton1";
            this->toolStripButton1->SettingsKey = L"MainForm.toolStripButton1";
            this->toolStripButton1->ToolTipText = L"Add Light";
            this->toolStripButton1->Click += gcnew System::EventHandler(this, &MainForm::toolStripButton1_Click);
            this->insertToolBarSep->Name = L"insertToolBarSep";
            this->insertToolBarSep->SettingsKey = L"MainForm.toolStripSeparator2";
            this->insertToolBarSep->Click += gcnew System::EventHandler(this, &MainForm::insertToolBarSep_Click);
            this->changeActor->Name = L"changeActor";
            this->changeActor->SettingsKey = L"MainForm.changeActor";
            this->changeActor->Text = L"Change";
            this->changeActor->ToolTipText = L"Changes selected actor to dropdown selection";
            this->changeActor->Click += gcnew System::EventHandler(this, &MainForm::changeActorButton_Click);
            this->addActorButton->Name = L"addActorButton";
            this->addActorButton->SettingsKey = L"MainForm.toolStripButton2";
            this->addActorButton->Text = L"Insert";
            this->addActorButton->ToolTipText = L"Inserts actor listed on dropdown";
            this->addActorButton->Click += gcnew System::EventHandler(this, &MainForm::addActorButton_Click);
            this->actorsCombo->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->actorsCombo->MaxDropDownItems = 15;
            this->actorsCombo->Name = L"actorsCombo";
            this->actorsCombo->SettingsKey = L"MainForm.toolStripComboBox1";
            this->actorsCombo->Size = System::Drawing::Size(99, 26);
            this->bottomRaftingContainer->Controls->Add(this->statusBar);
            this->bottomRaftingContainer->Dock = System::Windows::Forms::DockStyle::Bottom;
            this->bottomRaftingContainer->Name = L"bottomRaftingContainer";
            this->statusBar->BackColor = System::Drawing::SystemColors::Control;
            this->statusBar->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(3) {this->statusStripPanel1, this->toolStripSeparator3, this->toolStripSeparator6});
            this->statusBar->Location = System::Drawing::Point(0, 0);
            this->statusBar->Name = L"statusBar";
            this->statusBar->Padding = System::Windows::Forms::Padding(0, 0, 12, 0);
            this->statusBar->Raft = System::Windows::Forms::RaftingSides::Bottom;
            this->statusBar->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
            this->statusBar->TabIndex = 0;
            this->statusBar->Text = L"statusStrip1";
            this->statusStripPanel1->BorderSides = System::Windows::Forms::Border3DSide::Bottom;
            this->statusStripPanel1->Name = L"statusStripPanel1";
            this->statusStripPanel1->SettingsKey = L"MainForm.statusStripPanel1";
            this->statusStripPanel1->Text = L" ";
            this->statusStripPanel1->Click += gcnew System::EventHandler(this, &MainForm::statusStripPanel1_Click);
            this->toolStripSeparator3->Name = L"toolStripSeparator3";
            this->toolStripSeparator3->SettingsKey = L"MainForm.toolStripMenuItem3";
            this->toolStripSeparator6->Name = L"toolStripSeparator6";
            this->toolStripSeparator6->SettingsKey = L"MainForm.toolStripMenuItem3";
            this->importMeshDialog->DefaultExt = L"xml";
            this->importMeshDialog->Filter = L"Max/Maya XML and X-Files (*.x, *.xml) |*.x;*.xml|OBJ Format (*.obj)|*.obj|All fil" 
                L"es|*.*";
            this->importMeshDialog->InitialDirectory = L"..\\Models";
            this->importMeshDialog->Title = L"Import Mesh...";
            this->splitContainer1->BackColor = System::Drawing::SystemColors::GradientActiveCaption;
            this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer1->Location = System::Drawing::Point(0, 73);
            this->splitContainer1->Name = L"splitContainer1";
            this->splitContainer1->Panel1->Controls->Add(this->splitContainer2);
            this->splitContainer1->Panel2->BackColor = System::Drawing::SystemColors::Control;
            this->splitContainer1->Panel2->Controls->Add(this->tabPanelRight);
            this->splitContainer1->Panel2->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::splitContainer1_Panel2_Paint_1);
            this->splitContainer1->Size = System::Drawing::Size(1004, 581);
            this->splitContainer1->SplitterDistance = 780;
            this->splitContainer1->SplitterWidth = 2;
            this->splitContainer1->TabIndex = 14;
            this->splitContainer1->Text = L"splitContainer1";
            this->splitContainer2->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer2->Location = System::Drawing::Point(0, 0);
            this->splitContainer2->Name = L"splitContainer2";
            this->splitContainer2->Orientation = System::Windows::Forms::Orientation::Horizontal;
            this->splitContainer2->Panel1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
            this->splitContainer2->Panel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::splitContainer2_Panel1_Paint);
            this->splitContainer2->Panel1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::splitContainer2_Panel1_MouseMove);
            this->splitContainer2->Panel1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::splitContainer2_Panel1_MouseDown);
            this->splitContainer2->Panel2->BackColor = System::Drawing::SystemColors::GradientInactiveCaption;
            this->splitContainer2->Panel2->Controls->Add(this->splitContainer3);
            this->splitContainer2->Size = System::Drawing::Size(780, 581);
            this->splitContainer2->SplitterDistance = 470;
            this->splitContainer2->SplitterWidth = 6;
            this->splitContainer2->TabIndex = 0;
            this->splitContainer2->Text = L"splitContainer2";
            this->splitContainer3->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer3->Location = System::Drawing::Point(0, 0);
            this->splitContainer3->Name = L"splitContainer3";
            this->splitContainer3->Panel1->Controls->Add(this->richTextBox1);
            this->splitContainer3->Panel2->Controls->Add(this->statisticsBox);
            this->splitContainer3->Size = System::Drawing::Size(780, 105);
            this->splitContainer3->SplitterDistance = 410;
            this->splitContainer3->TabIndex = 2;
            this->splitContainer3->Text = L"splitContainer3";
            this->richTextBox1->BackColor = System::Drawing::SystemColors::Window;
            this->richTextBox1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->richTextBox1->Location = System::Drawing::Point(0, 0);
            this->richTextBox1->Name = L"richTextBox1";
            this->richTextBox1->ReadOnly = true;
            this->richTextBox1->ScrollBars = System::Windows::Forms::RichTextBoxScrollBars::ForcedBoth;
            this->richTextBox1->Size = System::Drawing::Size(410, 105);
            this->richTextBox1->TabIndex = 1;
            this->richTextBox1->Text = L"";
            this->statisticsBox->AutoSize = false;
            this->statisticsBox->BackColor = System::Drawing::SystemColors::Window;
            this->statisticsBox->Dock = System::Windows::Forms::DockStyle::Fill;
            this->statisticsBox->Location = System::Drawing::Point(0, 0);
            this->statisticsBox->Multiline = true;
            this->statisticsBox->Name = L"statisticsBox";
            this->statisticsBox->ReadOnly = true;
            this->statisticsBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
            this->statisticsBox->Size = System::Drawing::Size(366, 105);
            this->statisticsBox->TabIndex = 2;
            this->tabPanelRight->Controls->Add(this->tabPage1);
            this->tabPanelRight->Controls->Add(this->tabPage2);
            this->tabPanelRight->Controls->Add(this->tabPage3);
            this->tabPanelRight->Controls->Add(this->tabPage4);
            this->tabPanelRight->Dock = System::Windows::Forms::DockStyle::Fill;
            this->tabPanelRight->Location = System::Drawing::Point(0, 0);
            this->tabPanelRight->Margin = System::Windows::Forms::Padding(0);
            this->tabPanelRight->Multiline = true;
            this->tabPanelRight->Name = L"tabPanelRight";
            this->tabPanelRight->Padding = System::Drawing::Point(0, 0);
            this->tabPanelRight->SelectedIndex = 0;
            this->tabPanelRight->Size = System::Drawing::Size(222, 581);
            this->tabPanelRight->TabIndex = 0;
            this->tabPage1->BackColor = System::Drawing::SystemColors::Control;
            this->tabPage1->Controls->Add(this->propertyGrid1);
            this->tabPage1->Controls->Add(this->panel1);
            this->tabPage1->Controls->Add(this->panel2);
            this->tabPage1->EnableVisualStyleBackground = false;
            this->tabPage1->ForeColor = System::Drawing::SystemColors::ControlText;
            this->tabPage1->Location = System::Drawing::Point(4, 40);
            this->tabPage1->Name = L"tabPage1";
            this->tabPage1->Padding = System::Windows::Forms::Padding(3);
            this->tabPage1->Size = System::Drawing::Size(214, 537);
            this->tabPage1->TabIndex = 0;
            this->tabPage1->Text = L"General";
            this->propertyGrid1->BackColor = System::Drawing::SystemColors::Control;
            this->propertyGrid1->CommandsVisibleIfAvailable = true;
            this->propertyGrid1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->propertyGrid1->Location = System::Drawing::Point(3, 3);
            this->propertyGrid1->Name = L"propertyGrid1";
            this->propertyGrid1->Size = System::Drawing::Size(208, 206);
            this->propertyGrid1->TabIndex = 2;
            this->panel1->Controls->Add(this->checkIsExculsion);
            this->panel1->Controls->Add(this->button4);
            this->panel1->Controls->Add(this->button3);
            this->panel1->Controls->Add(this->label7);
            this->panel1->Controls->Add(this->label6);
            this->panel1->Controls->Add(this->listExclusion);
            this->panel1->Controls->Add(this->listSceneMeshes1);
            this->panel1->Dock = System::Windows::Forms::DockStyle::Bottom;
            this->panel1->Location = System::Drawing::Point(3, 209);
            this->panel1->Name = L"panel1";
            this->panel1->Size = System::Drawing::Size(208, 204);
            this->panel1->TabIndex = 3;
            this->panel1->Visible = false;
            this->checkIsExculsion->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->checkIsExculsion->AutoSize = true;
            this->checkIsExculsion->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->checkIsExculsion->Location = System::Drawing::Point(7, 183);
            this->checkIsExculsion->Name = L"checkIsExculsion";
            this->checkIsExculsion->Size = System::Drawing::Size(95, 17);
            this->checkIsExculsion->TabIndex = 6;
            this->checkIsExculsion->Text = L"Is Exculsion List";
            this->checkIsExculsion->CheckedChanged += gcnew System::EventHandler(this, &MainForm::checkIsExculsion_CheckedChanged);
            this->button4->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->button4->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button4->Location = System::Drawing::Point(109, 153);
            this->button4->Margin = System::Windows::Forms::Padding(0, 3, 3, 3);
            this->button4->Name = L"button4";
            this->button4->Size = System::Drawing::Size(91, 24);
            this->button4->TabIndex = 5;
            this->button4->Text = L"<< Remove";
            this->button4->Click += gcnew System::EventHandler(this, &MainForm::button4_Click);
            this->button3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->button3->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button3->Location = System::Drawing::Point(7, 153);
            this->button3->Margin = System::Windows::Forms::Padding(3, 2, 1, 3);
            this->button3->Name = L"button3";
            this->button3->Size = System::Drawing::Size(95, 23);
            this->button3->TabIndex = 4;
            this->button3->Text = L"Add >>";
            this->button3->Click += gcnew System::EventHandler(this, &MainForm::button3_Click);
            this->label7->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->label7->AutoSize = true;
            this->label7->Location = System::Drawing::Point(117, 7);
            this->label7->Name = L"label7";
            this->label7->Size = System::Drawing::Size(53, 14);
            this->label7->TabIndex = 3;
            this->label7->Text = L"Mesh List";
            this->label6->AutoSize = true;
            this->label6->Location = System::Drawing::Point(7, 7);
            this->label6->Name = L"label6";
            this->label6->Size = System::Drawing::Size(79, 14);
            this->label6->TabIndex = 2;
            this->label6->Text = L"Scene Meshes";
            this->listExclusion->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->listExclusion->FormattingEnabled = true;
            this->listExclusion->HorizontalScrollbar = true;
            this->listExclusion->Location = System::Drawing::Point(109, 26);
            this->listExclusion->Name = L"listExclusion";
            this->listExclusion->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
            this->listExclusion->Size = System::Drawing::Size(91, 121);
            this->listExclusion->TabIndex = 1;
            this->listSceneMeshes1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->listSceneMeshes1->FormattingEnabled = true;
            this->listSceneMeshes1->HorizontalScrollbar = true;
            this->listSceneMeshes1->Location = System::Drawing::Point(7, 26);
            this->listSceneMeshes1->Name = L"listSceneMeshes1";
            this->listSceneMeshes1->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
            this->listSceneMeshes1->Size = System::Drawing::Size(95, 121);
            this->listSceneMeshes1->TabIndex = 0;
            this->panel2->Controls->Add(this->label9);
            this->panel2->Controls->Add(this->panel3);
            this->panel2->Dock = System::Windows::Forms::DockStyle::Bottom;
            this->panel2->Location = System::Drawing::Point(3, 413);
            this->panel2->Name = L"panel2";
            this->panel2->Size = System::Drawing::Size(208, 121);
            this->panel2->TabIndex = 4;
            this->label9->AutoSize = true;
            this->label9->Location = System::Drawing::Point(5, 24);
            this->label9->Name = L"label9";
            this->label9->Size = System::Drawing::Size(0, 0);
            this->label9->TabIndex = 1;
            this->label9->Tag = L" ";
            this->panel3->BackColor = System::Drawing::SystemColors::ControlDarkDark;
            this->panel3->Controls->Add(this->label8);
            this->panel3->Controls->Add(this->linkLabel1);
            this->panel3->Dock = System::Windows::Forms::DockStyle::Top;
            this->panel3->Location = System::Drawing::Point(0, 0);
            this->panel3->Name = L"panel3";
            this->panel3->Size = System::Drawing::Size(208, 18);
            this->panel3->TabIndex = 0;
            this->label8->AutoSize = true;
            this->label8->ForeColor = System::Drawing::SystemColors::ButtonHighlight;
            this->label8->Location = System::Drawing::Point(7, 3);
            this->label8->Name = L"label8";
            this->label8->Size = System::Drawing::Size(54, 14);
            this->label8->TabIndex = 1;
            this->label8->Text = L"Mesh Info";
            this->linkLabel1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->linkLabel1->AutoSize = true;
            this->linkLabel1->LinkColor = System::Drawing::SystemColors::ButtonHighlight;
            this->linkLabel1->Links->Add(gcnew System::Windows::Forms::LinkLabel::Link(0, 4));
            this->linkLabel1->Location = System::Drawing::Point(174, 2);
            this->linkLabel1->Name = L"linkLabel1";
            this->linkLabel1->Size = System::Drawing::Size(28, 14);
            this->linkLabel1->TabIndex = 0;
            this->linkLabel1->TabStop = true;
            this->linkLabel1->Text = L"Hide";
            this->linkLabel1->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &MainForm::linkLabel1_LinkClicked);
            this->tabPage2->BackColor = System::Drawing::SystemColors::Control;
            this->tabPage2->Controls->Add(this->splitContainer4);
            this->tabPage2->EnableVisualStyleBackground = false;
            this->tabPage2->ForeColor = System::Drawing::SystemColors::ControlText;
            this->tabPage2->Location = System::Drawing::Point(4, 40);
            this->tabPage2->Margin = System::Windows::Forms::Padding(0);
            this->tabPage2->Name = L"tabPage2";
            this->tabPage2->Size = System::Drawing::Size(214, 537);
            this->tabPage2->TabIndex = 1;
            this->tabPage2->Text = L"Rendering";
            this->tabPage2->Click += gcnew System::EventHandler(this, &MainForm::tabPage2_Click);
            this->splitContainer4->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer4->Location = System::Drawing::Point(0, 0);
            this->splitContainer4->Name = L"splitContainer4";
            this->splitContainer4->Orientation = System::Windows::Forms::Orientation::Horizontal;
            this->splitContainer4->Panel1->Controls->Add(this->comboSubmat);
            this->splitContainer4->Panel1->Controls->Add(this->labelSubmat);
            this->splitContainer4->Panel1->Controls->Add(this->label2);
            this->splitContainer4->Panel1->Controls->Add(this->comboSubmesh);
            this->splitContainer4->Panel1->Controls->Add(this->renderProps);
            this->splitContainer4->Panel2->AutoScroll = true;
            this->splitContainer4->Panel2->Controls->Add(this->label5);
            this->splitContainer4->Panel2->Controls->Add(this->label1);
            this->splitContainer4->Panel2->Controls->Add(this->button2);
            this->splitContainer4->Panel2->Controls->Add(this->button1);
            this->splitContainer4->Panel2->Controls->Add(this->listBoxScene);
            this->splitContainer4->Panel2->Controls->Add(this->listBoxBlockers);
            this->splitContainer4->Size = System::Drawing::Size(214, 537);
            this->splitContainer4->SplitterDistance = 315;
            this->splitContainer4->TabIndex = 24;
            this->splitContainer4->Text = L"splitContainer4";
            this->comboSubmat->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->comboSubmat->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->comboSubmat->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->comboSubmat->FormattingEnabled = true;
            this->comboSubmat->Location = System::Drawing::Point(61, 37);
            this->comboSubmat->Margin = System::Windows::Forms::Padding(3, 1, 3, 0);
            this->comboSubmat->Name = L"comboSubmat";
            this->comboSubmat->Size = System::Drawing::Size(147, 21);
            this->comboSubmat->TabIndex = 20;
            this->labelSubmat->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->labelSubmat->AutoSize = true;
            this->labelSubmat->Location = System::Drawing::Point(7, 40);
            this->labelSubmat->Name = L"labelSubmat";
            this->labelSubmat->Size = System::Drawing::Size(46, 14);
            this->labelSubmat->TabIndex = 19;
            this->labelSubmat->Text = L"Submat:";
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(4, 14);
            this->label2->Margin = System::Windows::Forms::Padding(3, 3, 1, 3);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(55, 14);
            this->label2->TabIndex = 18;
            this->label2->Text = L"Submesh:";
            this->comboSubmesh->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->comboSubmesh->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->comboSubmesh->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->comboSubmesh->FormattingEnabled = true;
            this->comboSubmesh->Location = System::Drawing::Point(61, 11);
            this->comboSubmesh->Margin = System::Windows::Forms::Padding(1, 1, 3, 3);
            this->comboSubmesh->Name = L"comboSubmesh";
            this->comboSubmesh->Size = System::Drawing::Size(147, 21);
            this->comboSubmesh->TabIndex = 17;
            this->comboSubmesh->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::comboSubmesh_SelectedIndexChanged);
            this->renderProps->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->renderProps->BackColor = System::Drawing::SystemColors::Control;
            this->renderProps->CommandsVisibleIfAvailable = true;
            this->renderProps->Location = System::Drawing::Point(0, 59);
            this->renderProps->Margin = System::Windows::Forms::Padding(3, 1, 3, 3);
            this->renderProps->Name = L"renderProps";
            this->renderProps->Size = System::Drawing::Size(211, 255);
            this->renderProps->TabIndex = 16;
            this->renderProps->ToolbarVisible = false;
            this->label5->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->label5->AutoSize = true;
            this->label5->Location = System::Drawing::Point(111, 7);
            this->label5->Name = L"label5";
            this->label5->Size = System::Drawing::Size(85, 14);
            this->label5->TabIndex = 37;
            this->label5->Text = L"Blocker Meshes";
            this->label1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(14, 3);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(79, 14);
            this->label1->TabIndex = 36;
            this->label1->Text = L"Scene Meshes";
            this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->button2->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button2->Location = System::Drawing::Point(111, 184);
            this->button2->Name = L"button2";
            this->button2->Size = System::Drawing::Size(95, 24);
            this->button2->TabIndex = 35;
            this->button2->Text = L"<< Remove";
            this->button2->Click += gcnew System::EventHandler(this, &MainForm::buttonRemoveBlockers_Click);
            this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->button1->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button1->Location = System::Drawing::Point(8, 184);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(94, 24);
            this->button1->TabIndex = 34;
            this->button1->Text = L"Add >>";
            this->button1->Click += gcnew System::EventHandler(this, &MainForm::buttonAddBlockers_Click);
            this->listBoxScene->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->listBoxScene->FormattingEnabled = true;
            this->listBoxScene->HorizontalScrollbar = true;
            this->listBoxScene->Location = System::Drawing::Point(8, 26);
            this->listBoxScene->Margin = System::Windows::Forms::Padding(3, 1, 3, 3);
            this->listBoxScene->Name = L"listBoxScene";
            this->listBoxScene->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
            this->listBoxScene->Size = System::Drawing::Size(94, 147);
            this->listBoxScene->TabIndex = 33;
            this->listBoxBlockers->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->listBoxBlockers->FormattingEnabled = true;
            this->listBoxBlockers->HorizontalScrollbar = true;
            this->listBoxBlockers->Location = System::Drawing::Point(110, 26);
            this->listBoxBlockers->Margin = System::Windows::Forms::Padding(3, 1, 3, 3);
            this->listBoxBlockers->Name = L"listBoxBlockers";
            this->listBoxBlockers->SelectionMode = System::Windows::Forms::SelectionMode::MultiExtended;
            this->listBoxBlockers->Size = System::Drawing::Size(96, 147);
            this->listBoxBlockers->TabIndex = 31;
            this->tabPage3->Controls->Add(this->panel4);
            this->tabPage3->Location = System::Drawing::Point(4, 40);
            this->tabPage3->Name = L"tabPage3";
            this->tabPage3->Size = System::Drawing::Size(214, 537);
            this->tabPage3->TabIndex = 2;
            this->tabPage3->Text = L"Selection Lists";
            this->panel4->Controls->Add(this->button7);
            this->panel4->Controls->Add(this->clearListButton);
            this->panel4->Controls->Add(this->button9);
            this->panel4->Controls->Add(this->button8);
            this->panel4->Controls->Add(this->selectedListBox);
            this->panel4->Controls->Add(this->button6);
            this->panel4->Controls->Add(this->button5);
            this->panel4->Controls->Add(this->selectedListComboBox);
            this->panel4->Controls->Add(this->label10);
            this->panel4->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel4->Location = System::Drawing::Point(0, 0);
            this->panel4->Name = L"panel4";
            this->panel4->Size = System::Drawing::Size(214, 537);
            this->panel4->TabIndex = 0;
            this->panel4->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::panel4_Paint);
            this->button7->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->button7->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button7->Location = System::Drawing::Point(8, 466);
            this->button7->Margin = System::Windows::Forms::Padding(3, 3, 0, 3);
            this->button7->Name = L"button7";
            this->button7->Size = System::Drawing::Size(121, 27);
            this->button7->TabIndex = 13;
            this->button7->Text = L"<< List To Selected";
            this->button7->Click += gcnew System::EventHandler(this, &MainForm::button7_Click);
            this->clearListButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->clearListButton->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->clearListButton->Location = System::Drawing::Point(136, 466);
            this->clearListButton->Margin = System::Windows::Forms::Padding(1, 3, 3, 3);
            this->clearListButton->Name = L"clearListButton";
            this->clearListButton->Size = System::Drawing::Size(75, 27);
            this->clearListButton->TabIndex = 16;
            this->clearListButton->Text = L"< Clear List >";
            this->clearListButton->Click += gcnew System::EventHandler(this, &MainForm::clearListButton_Click);
            this->button9->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->button9->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button9->Location = System::Drawing::Point(150, 27);
            this->button9->Margin = System::Windows::Forms::Padding(1, 3, 3, 3);
            this->button9->Name = L"button9";
            this->button9->Size = System::Drawing::Size(57, 21);
            this->button9->TabIndex = 15;
            this->button9->Text = L"Rename";
            this->button9->Click += gcnew System::EventHandler(this, &MainForm::button9_Click);
            this->button8->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->button8->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button8->Location = System::Drawing::Point(8, 503);
            this->button8->Margin = System::Windows::Forms::Padding(1, 3, 3, 3);
            this->button8->Name = L"button8";
            this->button8->Size = System::Drawing::Size(121, 27);
            this->button8->TabIndex = 14;
            this->button8->Text = L"Selected To List >>";
            this->button8->Click += gcnew System::EventHandler(this, &MainForm::button8_Click);
            this->selectedListBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->selectedListBox->FormattingEnabled = true;
            this->selectedListBox->Location = System::Drawing::Point(7, 88);
            this->selectedListBox->Margin = System::Windows::Forms::Padding(3, 3, 3, 1);
            this->selectedListBox->Name = L"selectedListBox";
            this->selectedListBox->Size = System::Drawing::Size(200, 342);
            this->selectedListBox->TabIndex = 12;
            this->selectedListBox->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::selectedListBox_SelectedIndexChanged);
            this->button6->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->button6->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button6->Location = System::Drawing::Point(128, 58);
            this->button6->Name = L"button6";
            this->button6->Size = System::Drawing::Size(79, 23);
            this->button6->TabIndex = 11;
            this->button6->Text = L"Remove List";
            this->button6->Click += gcnew System::EventHandler(this, &MainForm::button6_Click);
            this->button5->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->button5->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button5->Location = System::Drawing::Point(7, 58);
            this->button5->Name = L"button5";
            this->button5->Size = System::Drawing::Size(114, 23);
            this->button5->TabIndex = 10;
            this->button5->Text = L"Add List";
            this->button5->Click += gcnew System::EventHandler(this, &MainForm::button5_Click);
            this->selectedListComboBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->selectedListComboBox->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->selectedListComboBox->FormattingEnabled = true;
            this->selectedListComboBox->Location = System::Drawing::Point(7, 27);
            this->selectedListComboBox->Margin = System::Windows::Forms::Padding(3, 3, 2, 3);
            this->selectedListComboBox->Name = L"selectedListComboBox";
            this->selectedListComboBox->Size = System::Drawing::Size(135, 21);
            this->selectedListComboBox->TabIndex = 9;
            this->selectedListComboBox->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::selectedListComboBox_SelectedIndexChanged);
            this->selectedListComboBox->TextUpdate += gcnew System::EventHandler(this, &MainForm::selectedListComboBox_TextUpdate);
            this->label10->AutoSize = true;
            this->label10->BackColor = System::Drawing::Color::Transparent;
            this->label10->Location = System::Drawing::Point(7, 6);
            this->label10->Name = L"label10";
            this->label10->Size = System::Drawing::Size(56, 14);
            this->label10->TabIndex = 8;
            this->label10->Text = L"Select List";
            this->tabPage4->Controls->Add(this->statsText);
            this->tabPage4->Location = System::Drawing::Point(4, 40);
            this->tabPage4->Name = L"tabPage4";
            this->tabPage4->Padding = System::Windows::Forms::Padding(3);
            this->tabPage4->Size = System::Drawing::Size(214, 537);
            this->tabPage4->TabIndex = 3;
            this->tabPage4->Text = L"Stats";
            this->statsText->Dock = System::Windows::Forms::DockStyle::Fill;
            this->statsText->Location = System::Drawing::Point(3, 3);
            this->statsText->Multiline = true;
            this->statsText->Name = L"statsText";
            this->statsText->ReadOnly = true;
            this->statsText->Size = System::Drawing::Size(208, 531);
            this->statsText->TabIndex = 0;
            this->saveFileDialog->DefaultExt = L"xml";
            this->saveFileDialog->Filter = L"XML|*.xml|All files|*.*";
            this->saveFileDialog->InitialDirectory = L"..";
            this->contextMenuStrip1->AllowDrop = true;
            this->contextMenuStrip1->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(5) {this->testToolStripMenuItem, this->hideUnSelectedToolStripMenuItem1, this->unhideAllToolStripMenuItem1, this->freezeSelectedToolStripMenuItem1, this->unFreezeAllToolStripMenuItem1});
            this->contextMenuStrip1->Location = System::Drawing::Point(45, 54);
            this->contextMenuStrip1->Name = L"contextMenuStrip1";
            this->contextMenuStrip1->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
            this->contextMenuStrip1->ShowImageMargin = false;
            this->contextMenuStrip1->Size = System::Drawing::Size(117, 114);
            this->contextMenuStrip1->Click += gcnew System::EventHandler(this, &MainForm::contextMenuStrip1_Click);
            this->testToolStripMenuItem->Name = L"testToolStripMenuItem";
            this->testToolStripMenuItem->SettingsKey = L"MainForm.testToolStripMenuItem";
            this->testToolStripMenuItem->Text = L"Hide Selected";
            this->testToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::hideSelectedToolStripMenuItem_Click);
            this->hideUnSelectedToolStripMenuItem1->Name = L"hideUnSelectedToolStripMenuItem1";
            this->hideUnSelectedToolStripMenuItem1->SettingsKey = L"MainForm.hideUnSelectedToolStripMenuItem1";
            this->hideUnSelectedToolStripMenuItem1->Text = L"Hide UnSelected";
            this->hideUnSelectedToolStripMenuItem1->Click += gcnew System::EventHandler(this, &MainForm::hideUnselectedToolStripMenuItem_Click);
            this->unhideAllToolStripMenuItem1->Name = L"unhideAllToolStripMenuItem1";
            this->unhideAllToolStripMenuItem1->SettingsKey = L"MainForm.unhideAllToolStripMenuItem1";
            this->unhideAllToolStripMenuItem1->Text = L"Unhide All";
            this->unhideAllToolStripMenuItem1->Click += gcnew System::EventHandler(this, &MainForm::unHideAllToolStripMenuItem_Click);
            this->freezeSelectedToolStripMenuItem1->Name = L"freezeSelectedToolStripMenuItem1";
            this->freezeSelectedToolStripMenuItem1->SettingsKey = L"MainForm.freezeSelectedToolStripMenuItem1";
            this->freezeSelectedToolStripMenuItem1->Text = L"Freeze Selected";
            this->freezeSelectedToolStripMenuItem1->Click += gcnew System::EventHandler(this, &MainForm::freezeSelectedToolStripMenuItem_Click);
            this->unFreezeAllToolStripMenuItem1->Name = L"unFreezeAllToolStripMenuItem1";
            this->unFreezeAllToolStripMenuItem1->SettingsKey = L"MainForm.unFreezeAllToolStripMenuItem1";
            this->unFreezeAllToolStripMenuItem1->Text = L"UnFreeze All";
            this->unFreezeAllToolStripMenuItem1->Click += gcnew System::EventHandler(this, &MainForm::unFreezeAllToolStripMenuItem_Click);
            this->buttonRemoveBlockers->Location = System::Drawing::Point(81, 114);
            this->buttonRemoveBlockers->Name = L"buttonRemoveBlockers";
            this->buttonRemoveBlockers->TabIndex = 35;
            this->buttonRemoveBlockers->Text = L"<< Remove";
            this->buttonRemoveBlockers->Click += gcnew System::EventHandler(this, &MainForm::buttonRemoveBlockers_Click);
            this->label4->AutoSize = true;
            this->label4->Location = System::Drawing::Point(71, 57);
            this->label4->Name = L"label4";
            this->label4->Size = System::Drawing::Size(85, 14);
            this->label4->TabIndex = 34;
            this->label4->Text = L"Blocker Meshes";
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(26, 57);
            this->label3->Margin = System::Windows::Forms::Padding(3, 3, 3, 1);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(79, 14);
            this->label3->TabIndex = 32;
            this->label3->Text = L"Scene Meshes";
            this->buttonAddBlockers->Location = System::Drawing::Point(22, 114);
            this->buttonAddBlockers->Name = L"buttonAddBlockers";
            this->buttonAddBlockers->TabIndex = 30;
            this->buttonAddBlockers->Text = L"Add >>";
            this->buttonAddBlockers->Click += gcnew System::EventHandler(this, &MainForm::buttonAddBlockers_Click);
            this->exportFileDialog->DefaultExt = L"obj";
            this->exportFileDialog->Filter = L"OBJ Files|*.obj|All files|*.*";
            this->exportFileDialog->InitialDirectory = L"..\\Models\\";
            this->exportFileDialog->Title = L"Export Selected To...";
            this->openFileDialog1->DefaultExt = L"xml";
            this->openFileDialog1->Filter = L"XML Files (*.xml) |*.xml|All files|*.*";
            this->openFileDialog1->InitialDirectory = L"..\\Maps";
            this->openFileDialog1->Title = L"Open Scene";
            this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
            this->ClientSize = System::Drawing::Size(1004, 673);
            this->Controls->Add(this->splitContainer1);
            this->Controls->Add(this->leftRaftingContainer);
            this->Controls->Add(this->rightRaftingContainer);
            this->Controls->Add(this->topRaftingContainer);
            this->Controls->Add(this->bottomRaftingContainer);
            this->Icon = (stdcli::language::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->Name = L"MainForm";
            this->Text = L"Reality Builder";
            this->Load += gcnew System::EventHandler(this, &MainForm::Form1_Load);
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->mainToolbar))->LoadComponentSettings();
            this->mainToolbar->ResumeLayout(false);
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer))->EndInit();
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->toolsToolbar))->LoadComponentSettings();
            this->toolsToolbar->ResumeLayout(false);
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->rightRaftingContainer))->EndInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->topRaftingContainer))->EndInit();
            this->topRaftingContainer->ResumeLayout(false);
            this->topRaftingContainer->PerformLayout();
            this->mainMenu->ResumeLayout(false);
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->helpToolStripMenuItem))->LoadComponentSettings();
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->toolStrip3))->LoadComponentSettings();
            this->toolStrip3->ResumeLayout(false);
            this->toolStrip3->PerformLayout();
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->toolStrip2))->LoadComponentSettings();
            this->toolStrip2->ResumeLayout(false);
            this->toolStrip2->PerformLayout();
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->sTGEditBoxToolStripButton))->LoadComponentSettings();
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->toolStrip1))->LoadComponentSettings();
            this->toolStrip1->ResumeLayout(false);
            this->toolStrip1->PerformLayout();
            (stdcli::language::safe_cast<System::Configuration::IPersistComponentSettings^  >(this->consoleToolStripTextBox))->LoadComponentSettings();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bottomRaftingContainer))->EndInit();
            this->bottomRaftingContainer->ResumeLayout(false);
            this->bottomRaftingContainer->PerformLayout();
            this->statusBar->ResumeLayout(false);
            this->splitContainer1->Panel1->ResumeLayout(false);
            this->splitContainer1->Panel2->ResumeLayout(false);
            this->splitContainer1->ResumeLayout(false);
            this->splitContainer2->Panel2->ResumeLayout(false);
            this->splitContainer2->ResumeLayout(false);
            this->splitContainer3->Panel1->ResumeLayout(false);
            this->splitContainer3->Panel2->ResumeLayout(false);
            this->splitContainer3->ResumeLayout(false);
            this->tabPanelRight->ResumeLayout(false);
            this->tabPage1->ResumeLayout(false);
            this->panel1->ResumeLayout(false);
            this->panel1->PerformLayout();
            this->panel2->ResumeLayout(false);
            this->panel2->PerformLayout();
            this->panel3->ResumeLayout(false);
            this->panel3->PerformLayout();
            this->tabPage2->ResumeLayout(false);
            this->splitContainer4->Panel1->ResumeLayout(false);
            this->splitContainer4->Panel1->PerformLayout();
            this->splitContainer4->Panel2->ResumeLayout(false);
            this->splitContainer4->Panel2->PerformLayout();
            this->splitContainer4->ResumeLayout(false);
            this->tabPage3->ResumeLayout(false);
            this->panel4->ResumeLayout(false);
            this->panel4->PerformLayout();
            this->tabPage4->ResumeLayout(false);
            this->tabPage4->PerformLayout();
            this->contextMenuStrip1->ResumeLayout(false);
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion

    private : void FillSelectionLists()
              {
                  selectedListComboBox->Items->Clear();
                  //excSelectionList->Items->Clear();
                  for (int i=0;i<editor->m_ActorLists.size();i++)
                  {
                      String^ item=gcnew String(editor->m_ActorLists[i].ListName.c_str());
                      selectedListComboBox->Items->Add(item);
                      //excSelectionList->Items->Add(item);
                  }
                  //if (excSelectionList->Items->Count>0)
                  //  excSelectionList->SelectedIndex=0;
              }

              /// <summary>
              /// Finished editing X coordinate box
              /// </summary>
              void X_Leave(Object^ sender, EventArgs^ args)
              {
                  ToolStripTextBox^ box = (ToolStripTextBox^)sender;
                  if(box->Text->Length == 0)
                      return;
                  double newValue;
                  try{
                      newValue = Convert::ToDouble(box->Text,Globalization::CultureInfo::CurrentCulture);
                  }
                  catch(...){
                      return; // Invalid number
                  }

                  if(moveButton->Checked)
                  {
                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                      {
                          editor->m_SelectedActors[i]->Location.x = newValue;
                      }
                  } 
                  else if(rotateButton->Checked && editor->m_SelectedActors.size() > 0)
                  {
                      Matrix newRotation;	
                      newRotation.SetRotations(newValue/PICONVERSION, 0, 0); 


                      Vector RotCenter(0,0,0);
                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                          RotCenter += (editor->m_SelectedActors[i]->Location / 						  
                          editor->m_SelectedActors.size());						  

                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                      {
                          Actor* actor = editor->m_SelectedActors[i];

                          // Multiple objects pivot around group center. Single objects pivot around themselves
                          if(editor->m_SelectedActors.size() > 1)
                          {
                              Matrix RotCenterMat;							
                              RotCenterMat.SetTranslations(RotCenter.x,RotCenter.y,RotCenter.z);
                              Matrix ActorCenterMat;

                              ActorCenterMat.SetTranslations(actor->Location.x,actor->Location.y,actor->Location.z);

                              Matrix mActor = actor->Rotation;
                              mActor[3] = actor->Location;

                              mActor = mActor * (RotCenterMat.Inverse() * newRotation * RotCenterMat);

                              actor->Rotation = mActor.GetRotationMatrix();
                              actor->Location = mActor[3];
                          }
                          else
                          {
                              actor->Rotation.SetXRotation(newValue/PICONVERSION);							  
                          }	

                      };					  
                  } 
                  else if(scaleButton->Checked && editor->m_SelectedActors.size()==1)
                  {		
                      Matrix newScale;
                      Vector currentScale;

                      if(editor->m_SelectedActors[0]->MyModel)
                      {
                          BBox currentScaleB = editor->m_SelectedActors[0]->MyModel->GetWorldBBox();

                          if(newValue != 0)
                          {
                              currentScale = currentScaleB.max - currentScaleB.min;
                              float newFScale = newValue / currentScale.x;
                              newScale.SetXScale(newFScale);

                              editor->m_SelectedActors[0]->Rotation = 
                                  editor->m_SelectedActors[0]->Rotation * newScale;
                          }

                      };
                  }	 else boxX->Text= " ";


              }

              /// <summary>
              /// Finished editing Y coordinate box
              /// </summary>
              void Y_Leave(Object^ sender, EventArgs^ args)
              {
                  ToolStripTextBox^ box = (ToolStripTextBox^)sender;
                  if(box->Text->Length == 0)
                      return;
                  double newValue;
                  try{
                      newValue = Convert::ToDouble(box->Text,Globalization::CultureInfo::CurrentCulture);
                  }
                  catch(...){
                      return; // Invalid number
                  }

                  if(moveButton->Checked)
                  {
                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                      {
                          editor->m_SelectedActors[i]->Location.y = newValue;
                      }
                  } 
                  else if(rotateButton->Checked && editor->m_SelectedActors.size() > 0)
                  {
                      Matrix newRotation;	
                      newRotation.SetRotations(0, newValue/PICONVERSION, 0); 

                      Vector RotCenter(0,0,0);
                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                          RotCenter += (editor->m_SelectedActors[i]->Location / 						  
                          editor->m_SelectedActors.size());						  

                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                      {
                          Actor* actor = editor->m_SelectedActors[i];

                          // Multiple objects pivot around group center. Single objects pivot around themselves
                          if(editor->m_SelectedActors.size() > 1)
                          {
                              Matrix RotCenterMat;							
                              RotCenterMat.SetTranslations(RotCenter.x,RotCenter.y,RotCenter.z);
                              Matrix ActorCenterMat;

                              ActorCenterMat.SetTranslations(actor->Location.x,actor->Location.y,actor->Location.z);

                              Matrix mActor = actor->Rotation;
                              mActor[3] = actor->Location;

                              mActor = mActor * (RotCenterMat.Inverse() * newRotation * RotCenterMat);

                              actor->Rotation = mActor.GetRotationMatrix();
                              actor->Location = mActor[3];
                          }
                          else
                          {
                              actor->Rotation.SetYRotation(newValue/PICONVERSION);
                          }	

                      };					  
                  }
                  else if(scaleButton->Checked && editor->m_SelectedActors.size()==1)
                  {		
                      Matrix newScale;
                      Vector currentScale;

                      if(editor->m_SelectedActors[0]->MyModel)
                      {
                          BBox currentScaleB = editor->m_SelectedActors[0]->MyModel->GetWorldBBox();

                          if(newValue != 0)
                          {
                              currentScale = currentScaleB.max - currentScaleB.min;
                              float newFScale = newValue / currentScale.y;
                              newScale.SetYScale(newFScale);

                              editor->m_SelectedActors[0]->Rotation = 
                                  editor->m_SelectedActors[0]->Rotation * newScale;
                          }

                      };
                  }	 else boxY->Text= " ";


              }

              /// <summary>
              /// Finished editing Z coordinate box
              /// </summary>
              void Z_Leave(Object^ sender, EventArgs^ args)
              {
                  ToolStripTextBox^ box = (ToolStripTextBox^)sender;
                  if(box->Text->Length == 0)
                      return;
                  double newValue;
                  try{
                      newValue = Convert::ToDouble(box->Text,Globalization::CultureInfo::CurrentCulture);
                  }
                  catch(...){
                      return; // Invalid number
                  }

                  if(moveButton->Checked)
                  {
                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                      {
                          editor->m_SelectedActors[i]->Location.z = newValue;
                      }
                  } 
                  else if(rotateButton->Checked && editor->m_SelectedActors.size() > 0)
                  {
                      Matrix newRotation;	
                      newRotation.SetRotations(0, 0, newValue/PICONVERSION); 

                      Vector RotCenter(0,0,0);
                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                          RotCenter += (editor->m_SelectedActors[i]->Location / 						  
                          editor->m_SelectedActors.size());						  

                      for(int i=0;i<editor->m_SelectedActors.size();i++)
                      {
                          Actor* actor = editor->m_SelectedActors[i];

                          // Multiple objects pivot around group center. Single objects pivot around themselves
                          if(editor->m_SelectedActors.size() > 1)
                          {
                              Matrix RotCenterMat;							
                              RotCenterMat.SetTranslations(RotCenter.x,RotCenter.y,RotCenter.z);
                              Matrix ActorCenterMat;

                              ActorCenterMat.SetTranslations(actor->Location.x,actor->Location.y,actor->Location.z);

                              Matrix mActor = actor->Rotation;
                              mActor[3] = actor->Location;

                              mActor = mActor * (RotCenterMat.Inverse() * newRotation * RotCenterMat);

                              actor->Rotation = mActor.GetRotationMatrix();
                              actor->Location = mActor[3];
                          }
                          else
                          {
                              actor->Rotation.SetZRotation(newValue/PICONVERSION);
                          }	

                      };					  
                  }			
                  else if(scaleButton->Checked && editor->m_SelectedActors.size()==1)
                  {		
                      Matrix newScale;
                      Vector currentScale;

                      if(editor->m_SelectedActors[0]->MyModel)
                      {
                          BBox currentScaleB = editor->m_SelectedActors[0]->MyModel->GetWorldBBox();

                          if(newValue != 0)
                          {
                              currentScale = currentScaleB.max - currentScaleB.min;
                              float newFScale = newValue / currentScale.z;
                              newScale.SetZScale(newFScale);

                              editor->m_SelectedActors[0]->Rotation = 
                                  editor->m_SelectedActors[0]->Rotation * newScale;
                          }

                      };
                  }	 else boxZ->Text= " ";



              }

              /// <summary>
              /// Finished editing Snap To Grid value
              /// </summary>
              void STG_Leave(Object^ sender, EventArgs^ args)
              {
                  ToolStripTextBox^ box = (ToolStripTextBox^)sender;
                  double newValue = Convert::ToDouble(box->Text,Globalization::CultureInfo::CurrentCulture);
                  editor->m_SnapToGridValue=newValue;
              }





              /// <summary>
              /// Adds an item to the MRU list
              /// </summary>
              void AddMRU(String^ file)
              {
                  ToolStripMenuItem^ item = gcnew ToolStripMenuItem;
                  item->Text = file;
                  item->Click += gcnew System::EventHandler(this, &MainForm::MRULoad);
                  // See if entry exists, if so remove it
                  for(int i=0;i<fileMenu->DropDownItems->Count;i++)
                  {
                      if(fileMenu->DropDownItems[i]->Text->Equals(file))
                          fileMenu->DropDownItems->RemoveAt(i);
                  }

                  // See if we've gone above 5 entries, if so remove the oldest
                  int count = 0;
                  for(int i=0;i<fileMenu->DropDownItems->Count;i++)
                  {
                      if(fileMenu->DropDownItems[i]->Text->Contains(".xml"))
                      {
                          count++;
                          if(count >= 5){
                              fileMenu->DropDownItems->RemoveAt(i);
                              i--;
                          }
                      }
                  }

                  // NOTE: 12 = index in menu, need to up if we add more menu entries
                  this->fileMenu->DropDownItems->Insert(12,item);


                  // Save most recent files
                  ConfigFile file;
                  file.Create("MRU.ini"); // Erase any old file
                  count = 0;
                  for(int i=fileMenu->DropDownItems->Count-1;i>=0;i--)
                  {
                      if(fileMenu->DropDownItems[i]->Text->Contains(".xml"))
                          file.SetString("MRU"+ToStr(count++),ToCppString(fileMenu->DropDownItems[i]->Text));
                  }
              }

              /// <summary>
              /// Helper: Encapsulates loading a new map
              /// </summary>
              void LoadMap(String^ str)
              {
                  editor->ClearUndos();
                  editor->UnSelectAll();

                  try
                  {
                      Game_NewMap(ToCppString(str).c_str());
                  }
                  catch (System::AccessViolationException^ ex)
                  {
                      System::Windows::Forms::MessageBox::Show("An access violation occured when loading the map. Attempting to recover...");
                  }

                  this->Text = String::Concat(str->Substring(str->LastIndexOf("\\")+1)," -- Reality Builder");
                  ResetActorsIDs();
                  FillSelectionLists();
              }

              /// <summary>
              /// Load item from most recently used list
              /// </summary>
              void MRULoad(Object^ sender, EventArgs^ args)
              {
                  ToolStripMenuItem^ item = (ToolStripMenuItem^)sender;//reinterpret_cast<ToolStripMenuItem^>(sender);
                  LoadMap(item->Text);
                  // Re-insert map at top of list
                  AddMRU(item->Text);
              }

              /// <summary>
              /// Console command processing
              /// </summary>
              void ConsoleCommand(Object^ /*sender*/, KeyEventArgs^ e) 
              {
                  if(e->KeyData == Keys::Enter || e->KeyData == Keys::Return)
                      Game_RunConsoleCommand(ToCppString(consoleToolStripTextBox->Text));
              }

              void Main_OnClose(Object^ sender, System::ComponentModel::CancelEventArgs^ args)
              {
                  editor->ClearUndos();
                  assetBrowser->Close();

                  // Initializes the variables to pass to the MessageBox::Show method.
                  /*String^ message = "Are you sure you wish to exit Reality Builder?";
                  String^ caption = "Really quit Reality Builder?";
                  MessageBoxButtons buttons = MessageBoxButtons::YesNo;
                  System::Windows::Forms::DialogResult result;

                  // Displays the MessageBox.
                  result = MessageBox::Show(this, message, caption, buttons,
                  MessageBoxIcon::Question, MessageBoxDefaultButton::Button1, 
                  MessageBoxOptions::RightAlign);

                  if (result == System::Windows::Forms::DialogResult::No) {
                  // Don't quit
                  args->Cancel = false;
                  }*/
              }

              /// <summary>
              /// Key Processing
              /// </summary>
              void TopView_KeyDown(Object^ /*sender*/, System::Windows::Forms::KeyEventArgs^ e) 
              {
                  // Handle user escaping out of game mode or compile mode
                  if(e->KeyData == Keys::Escape)
                  {
                      callback.bCancel = true;

                      if(!editor->GetEditorMode())
                      {
                          editor->SetEditorMode(true);
                          return;
                      }
                  }

                  // Handle keyboard messages in play mode
                  if(!editor->GetEditorMode())
                      Game_HandleMessages((HWND)this->splitContainer2->Panel1->Handle.ToInt32(),WM_CHAR,(WPARAM)e->KeyCode,(WPARAM)e->KeyData);

                  // Only process keys when editor render window is focus window
                  if(!m_bRunning || !this->splitContainer2->Panel1->Focused || !editor->GetEditorMode()) return;

                  // Delete key
                  if(e->KeyData == Keys::Delete)	
                  {
                      editor->DeleteSelected();			
                      UpdateSelectedList();
                  };

                  // Gizmo keys
                  if(e->KeyData == Keys::D1)
                      moveButton_Click(0,gcnew System::EventArgs());
                  if(e->KeyData == Keys::D2)
                      rotateButton_Click(0,gcnew System::EventArgs());
                  if(e->KeyData == Keys::D3)
                      scaleButton_Click(0,gcnew System::EventArgs());
              }


              /// TIM: Converting events to WM_ messages because the WndProc does not pickup WM_MOUSEMOVE etc
              /// Why not??? Would be much cleaner to pass the messages straight through
              void TopView_MouseMove(Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e) 
              {
                  if(!m_bRunning) return;

                  Game_HandleMessages((HWND)this->splitContainer2->Panel1->Handle.ToInt32(),WM_MOUSEMOVE,(WPARAM)0,MAKELPARAM(e->X,e->Y));
              }
              void TopView_MouseDown(Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e)
              {
                  // Give view window focus if clicked, to avoid typing in boxes when moving!!
                  if(e->X < splitContainer2->Panel1->Width && e->Y < splitContainer2->Panel1->Height)
                      splitContainer2->Panel1->Focus();

                  editor->SetFocus(splitContainer2->Panel1->Focused);
                  if(!m_bRunning) return;
                  DWORD msg = WM_LBUTTONDOWN;
                  if(e->Button == Windows::Forms::MouseButtons::Right)
                      msg = WM_RBUTTONDOWN;
                  if(e->Button == Windows::Forms::MouseButtons::Middle)
                      msg = WM_MBUTTONDOWN;

                  if(!bInGameDLL)
                  {
                      bInGameDLL = true;
                      Game_HandleMessages((HWND)this->splitContainer2->Panel1->Handle.ToInt32(),msg,(WPARAM)0,MAKELPARAM(e->X,e->Y));
                      bInGameDLL = false;
                  }
              }
              void TopView_MouseUp(Object^ /*sender*/, System::Windows::Forms::MouseEventArgs^ e)
              {
                  if(!m_bRunning) return;
                  DWORD msg = WM_LBUTTONUP;
                  if(e->Button == Windows::Forms::MouseButtons::Right)
                      msg = WM_RBUTTONUP;
                  if(e->Button == Windows::Forms::MouseButtons::Middle)
                      msg = WM_MBUTTONUP;

                  if(!bInGameDLL)
                  {
                      bInGameDLL = true;
                      Game_HandleMessages((HWND)this->splitContainer2->Panel1->Handle.ToInt32(),msg,(WPARAM)0,MAKELPARAM(e->X,e->Y));
                      bInGameDLL = false;
                  }
              }

    private: System::Void Form1_Load(System::Object ^  sender, System::EventArgs ^  e)
             {
                 label9->Text="No Info!";
             }

             public: void UpdateCompilerProgress()
                     {
                         // Compiling message to statusbar, including completion message
                         static bool wasCompiling = false;
                         if(editor->m_bCompiling){
                             statusStripPanel1->Text = gcnew System::String((editor->curObject + " -- " + editor->curMsg).c_str());
                             wasCompiling = true;
                         }
                         else if(wasCompiling)
                         {
                             statusStripPanel1->Text = "Completed Compiling!";
                             wasCompiling = false;
                         }
                     }
             //
             // Main Loop
             //
    protected: [System::Security::Permissions::PermissionSet(System::Security::Permissions::SecurityAction::Demand, Name="FullTrust")]
               virtual void WndProc(Message% m) 
               {
                   // Process messages for engine window
                   // We can't process all messages, or will conflict with main editor window
                   bool pass = (m.Msg == WM_SIZE) || (m.Msg == WM_ENTERSIZEMOVE) || (m.Msg == WM_GETMINMAXINFO) || (m.Msg == WM_EXITSIZEMOVE)
                       || (m.Msg == WM_CHAR) || (m.Msg == WM_KEYDOWN)	   || /*(m.Msg == WM_CLOSE) ||*/ (m.Msg == WM_PAINT);

                   if(m_bRunning  && LibsInitialized() && (pass || m.HWnd == this->splitContainer2->Panel1->Handle))
                   {
                       if(!bInGameDLL)
                       {
                           bInGameDLL = true;
                           Game_HandleMessages((HWND)m.HWnd.ToInt32(),m.Msg,(WPARAM)m.WParam.ToInt32(),(LPARAM)m.LParam.ToInt32());
                           bInGameDLL = false;
                       }
                   }

                   Form::WndProc(m);
               }


               /// <summary>
               /// Rendering properties tab page
               /// </summary>
               void UpdateRenderProps()
               {
                   if(selectedMeshes->size() == 0)
                       return;

                   if(selectedMeshes->size() < comboSubmesh->SelectedIndex+1)
                       comboSubmesh->SelectedIndex = 0;

                   Mesh* m = (*selectedMeshes)[comboSubmesh->SelectedIndex]->GetMesh();
                   if(!m)
                       return;

                   // Fill material list from ALL meshes
                   comboSubmat->Items->Clear();
                   for(int j=0;j<m->m_Materials.size();j++)
                   {
                       comboSubmat->Items->Add(gcnew String(m->m_Materials[j]->m_Name.c_str()));
                   }
                   if(m->m_Materials.size())
                        comboSubmat->SelectedIndex = 0;

                   /////////////////////////////////////
                   // Scene meshes (possible blockers)
                   /////////////////////////////////////
                   listBoxScene->Items->Clear();

                   //
                   // Create sorted list to store on
                   //
                   ArrayList^ sceneList = gcnew ArrayList();
       

                   // Show Lists First
                   for(int i=0;i<editor->m_ActorLists.size();i++)
                   {
                       char groupAdd[50];
                       sprintf(groupAdd,"{%s}",editor->m_ActorLists.at(i).ListName.c_str());
                       sceneList->Add(gcnew String(groupAdd));
                   };

                   // show Mode Frames now
                   vector<ModelFrame*> worldMeshes;
                   editor->m_World->EnumerateMeshes(worldMeshes); 
                   for(int i=0;i<worldMeshes.size();i++)
                   {
                       // Only add scene meshes not already on blocker list
                       bool found = false;
                       for(int j=0;j<m->m_SHOptions.InBlockers.size();j++){
                           if(m->m_SHOptions.InBlockers[j] == worldMeshes[i]->Name){
                               found = true;
                               break;
                           }
                       }
                       if(!found)
                           sceneList->Add(gcnew String(worldMeshes[i]->Name.c_str()));
                   }

                   // Sort list
                   sceneList->Sort();

                   for(int i=0;i<sceneList->Count;i++)
                   {
                       listBoxScene->Items->Add(sceneList->Item[i]);
                   }

                   if(listBoxScene->Items->Count)
                       listBoxScene->SelectedIndex = 0;

                   // Current indoor blockers
                   listBoxBlockers->Items->Clear();

                   ArrayList^ list = gcnew ArrayList(m->m_SHOptions.InBlockers.size());
                   // Sort first
                   for(int i=0;i<m->m_SHOptions.InBlockers.size();i++)
                       list->Add(gcnew String(m->m_SHOptions.InBlockers[i].c_str()));
                   list->Sort();
                   // Add
                   for(int i=0;i<m->m_SHOptions.InBlockers.size();i++)
                   {
                       listBoxBlockers->Items->Add(list->Item[i]);
                   }
                   if(m->m_SHOptions.InBlockers.size())
                       listBoxBlockers->SelectedIndex = 0;

                   // Fill rendering properties
                   propHelper->UpdateVars(m->m_SHOptions.EditorVars);

                   // Register all other selected meshes vars as dependents on the first mesh
                   for(int i=1;i<editor->m_SelectedActors.size();i++)
                   {
                       vector<ModelFrame*> meshes;
                       if(editor->m_SelectedActors[i]->MyModel)
                           editor->m_SelectedActors[i]->MyModel->m_pFrameRoot->EnumerateMeshes(meshes);
                       for(int j=0;j<meshes.size();j++)
                           propHelper->currentVars->push_back(&meshes[j]->GetMesh()->m_SHOptions.EditorVars);
                   }

                   renderProps->SelectedObject = propHelper->class1;
               }

    private: void UpdatePropTabs()
             {
                 // Record selected meshes
                 SAFE_DELETE(selectedMeshes);
                 selectedMeshes = new vector<ModelFrame*>;
                 if(editor->m_SelectedActors.size())
                 {
                     // If one mesh selected, get mesh subset
                     if(editor->m_SelectedActors.size() == 1 && editor->m_SelectedActors[0]->MyModel)
                     {
                         // TODO: Get specific subset from UI, not just all segments
                         if(editor->m_SelectedActors[0]->MyModel && editor->m_SelectedActors[0]->MyModel->m_pFrameRoot)
                             editor->m_SelectedActors[0]->MyModel->m_pFrameRoot->EnumerateMeshes(*selectedMeshes);
                     }
                     // Else get all meshes
                     else if(editor->m_SelectedActors.size() > 1)
                     {
                         for(int i=0;i<editor->m_SelectedActors.size();i++)
                         {
                             if(editor->m_SelectedActors[i]->MyModel && editor->m_SelectedActors[i]->MyModel->m_pFrameRoot)
                                 editor->m_SelectedActors[i]->MyModel->m_pFrameRoot->EnumerateMeshes(*selectedMeshes);
                         }
                     }

                     //
                     // Print out mesh statistics
                     //
                     Model* model = editor->m_SelectedActors[0]->MyModel;
                     if (model && model->m_pFrameRoot && !editor->m_SelectedActors[0]->IsLight())
                     {
                         vector<ModelFrame*> meshes;
                         model->m_pFrameRoot->EnumerateMeshes(meshes);
                         label9->Text=String::Concat("Model Name: " , gcnew String( model->m_FileName.c_str()));
                         label9->Text=String::Concat(label9->Text , "\nTouching Lights: " , model->m_TouchingLights.size());
                         if(model->m_TouchingLights.size())
                             label9->Text=String::Concat(label9->Text , "\n (");
                         for(int i=0;i<model->m_TouchingLights.size();i++)
                         {
                             label9->Text=String::Concat(label9->Text , gcnew String( model->m_TouchingLights[i]->m_Name.c_str() ) );
                             label9->Text=String::Concat(label9->Text , ", ");
                         }
                         if(model->m_TouchingLights.size())
                             label9->Text=String::Concat(label9->Text , ")\nMeshes Count: " , meshes.size());

                         int drawCalls = 0;
                         for(int i=0;i<meshes.size();i++)
                         {

                             if(meshes[i]->GetMesh(meshes[i]->m_CurrentLOD))
                                 drawCalls += meshes[i]->GetMesh(meshes[i]->m_CurrentLOD)->m_DrawCalls;
                         }

                         label9->Text=String::Concat(label9->Text , "\nDraw Calls: " , drawCalls);

                         int vCount=0, fCount = 0;

                         for (int i=0;i<meshes.size();i++)
                         {
                             if(meshes[i]->GetMesh(meshes[i]->m_CurrentLOD))
                             {
                                 vCount += meshes[i]->GetMesh(meshes[i]->m_CurrentLOD)->GetHardwareMesh()->GetNumVertices();
                                 fCount += meshes[i]->GetMesh(meshes[i]->m_CurrentLOD)->GetHardwareMesh()->GetNumFaces();
                             }
                         }
                         label9->Text=String::Concat(label9->Text , "\nVertices Count: " , vCount);
                         label9->Text=String::Concat(label9->Text , "\nFaces Count: " , fCount);
                     }
                     else
                         label9->Text="No Info!";
                 }

                 if(editor->m_SelectedActors.size())
                 {
                     statusStripPanel1->Text = String::Concat(editor->m_SelectedActors.size()," selected: ");
                 }
                 else
                     statusStripPanel1->Text = "Nothing Selected";

                 for(int i=0;i<editor->m_SelectedActors.size();i++)
                 {
                     statusStripPanel1->Text = String::Concat(statusStripPanel1->Text,gcnew System::String(editor->m_SelectedActors[i]->m_Name.c_str()));
                     if(i < editor->m_SelectedActors.size()-1)
                         statusStripPanel1->Text = String::Concat(statusStripPanel1->Text,", ");
                 }



                 //
                 //Inclusion / EXclusion
                 //
                 if(editor->m_SelectedActors.size() && editor->m_SelectedActors[0]->m_HasIncludeExcludeList)
                 {
                     Actor * actor =(Light*)editor->m_SelectedActors[0];
                     panel1->Visible=true;

                     // Fill the scene meshes list
                     //vector<ModelFrame*> worldMeshes;
                     //editor->m_World->EnumerateMeshes(worldMeshes);
                     listSceneMeshes1->Items->Clear();

                     //add selection lists at top
                     for(int i=0;i<editor->m_ActorLists.size();i++)
                     {
                         char groupAdd[50];
                         sprintf(groupAdd,"{%s}",editor->m_ActorLists.at(i).ListName.c_str());

                         listSceneMeshes1->Items->Add(gcnew String(groupAdd));

                     };

                     //
                     // Create sorted list to store on
                     //
                     ArrayList^ sceneList = gcnew ArrayList();

                     //add all meshes
                     for(int i=0;i<editor->m_World->m_Actors.size();i++)
                     {
                         bool found = false;
                         for(int j=0;j<actor->m_ExcludeList.size();j++)
                         {
                             if( actor->m_ExcludeList[j] == editor->m_World->m_Actors.at(i)->m_Name)
                                 //	 worldMeshes[i]->Name)
                             {
                                 found = true;
                                 break;
                             }
                         }
                         if(!found && !editor->m_World->m_Actors.at(i)->IsLight())
                             sceneList->Add(gcnew String(
                             editor->m_World->m_Actors.at(i)->m_Name.c_str()));
                     }

                     // Sort list
                     sceneList->Sort();

                     for(int i=0;i<sceneList->Count;i++)
                         listSceneMeshes1->Items->Add(sceneList->Item[i]);


                     if(listBoxScene->Items->Count)
                         listBoxScene->SelectedIndex = 0;

                     // Fill up the exculsion list
                     ArrayList^ exclusionList = gcnew ArrayList();
                     listExclusion->Items->Clear();
                     for(int j=0;j<actor->m_ExcludeList.size();j++)
                         exclusionList->Add(gcnew String(actor->m_ExcludeList[j].c_str()));

                     // Sort list
                     exclusionList->Sort();

                     for(int i=0;i<exclusionList->Count;i++)
                         listExclusion->Items->Add(exclusionList->Item[i]);

                     if(listExclusion->Items->Count)
                         listExclusion->SelectedIndex = 0;

                     checkIsExculsion->Checked=actor->m_IsExcludeList;
                 }
                 else
                     panel1->Visible=false;

                 listBoxBlockers->Items->Clear();

                 //
                 // Entity Properties Tab
                 //
                 if(tabPanelRight->SelectedIndex == 0)
                 {
                     if(editor->m_SelectedActors.size() && editor->m_SelectedActors[0]->IsScriptActor())
                     {
                         // If multiple objects selected, use SelectedObjects feature of PropertyGrid
                         if(editor->m_SelectedActors.size() > 1)
                         {
                            propertyGrid1->SelectedObjects->Clear();

                            int count = 0;
                            for(int i=0;i<editor->m_SelectedActors.size();i++)
                            {
                                if(editor->m_SelectedActors[i]->IsScriptActor())
                                    count++;
                            }

                            stdcli::language::array<Object^> ^ actors = gcnew stdcli::language::array<Object^>(count);
                            count = 0;
                            for(int i=0;i<editor->m_SelectedActors.size();i++)
                            {
                                if(editor->m_SelectedActors[i]->IsScriptActor())
                                    actors[count++] = (MActor::s_actors[editor->m_SelectedActors[i]->GetManagedIndex()]);
                            }

                            propertyGrid1->SelectedObjects = actors;
                         }
                         else
                            propertyGrid1->SelectedObject = MActor::s_actors[editor->m_SelectedActors[0]->GetManagedIndex()];
                     }
                     else
                     {
                         if(editor->m_SelectedActors.size() == 0)
                             propHelper->class1->Properties->Clear();
                         else
                             propHelper->UpdateVars(editor->m_SelectedActors[0]->EditorVars);
                         propertyGrid1->SelectedObject = propHelper->class1;
                     }

                     // Register all other selected actors vars as dependents on the first actor
                     for(int i=1;i<editor->m_SelectedActors.size();i++)
                     {
                         propHelper->currentVars->push_back(&editor->m_SelectedActors[i]->EditorVars);
                     }
                 }
                 //
                 // Rendering Properties Tab
                 //
                 else if(tabPanelRight->SelectedIndex == 1 &&  (*selectedMeshes).size())
                 {
                     // Some areas only support one mesh right now
                     // TODO: Add drop-down for multi meshes


                     // Fill material list from ALL meshes
                     comboSubmesh->Items->Clear();
                     for(int j=0;j<(*selectedMeshes).size();j++)
                     {
                        comboSubmesh->Items->Add(gcnew String((*selectedMeshes)[j]->Name.c_str()));
                     }
                     comboSubmesh->SelectedIndex = 0;

                     UpdateRenderProps();
                 }
                 else if(tabPanelRight->SelectedIndex == 2)
                 {

                 }
             }

    private: bool updatingUI;
    private: void UpdateUI()
             {
                 StartMiniTimer();

                 /// In Play! mode, viewport window must always have focus
                 if(!editor->GetEditorMode())
                     this->splitContainer2->Panel1->Focus();

                 assetBrowser->SetSubMatIndex(comboSubmat->SelectedIndex);
                 updatingUI = true;

                 // Selected info!
                 static int index = -1;
                 if(editor->SelectionChanged() || (tabPanelRight->SelectedIndex != index))
                 {
                     index = tabPanelRight->SelectedIndex;
                     UpdatePropTabs();

                     // X/Y/Z
                     Vector v = editor->m_Gizmo.GetLocation();

                     if(moveButton->Checked && editor->m_SelectedActors.size() > 0)
                     {   
                         boxX->Text=Convert::ToString(v.x);
                         boxY->Text=Convert::ToString(v.y);
                         boxZ->Text=Convert::ToString(v.z);
                     }
                     else if(rotateButton->Checked && editor->m_SelectedActors.size()==1)
                     {
                         float AngX = 0, AngY = 0, AngZ = 0;

                         editor->GrabAngles(AngX, AngY, AngZ);

                         boxX->Text=Convert::ToString(AngX*PICONVERSION);
                         boxY->Text=Convert::ToString(AngY*PICONVERSION);
                         boxZ->Text=Convert::ToString(AngZ*PICONVERSION);
                     }
                     else if(scaleButton->Checked && editor->m_SelectedActors.size()>0)
                     {	
                         BBox MaxScale;
                         for(int i=0;i<editor->m_SelectedActors.size();i++)
                         {
                             if(editor->m_SelectedActors[i]->MyModel)
                                 MaxScale += editor->m_SelectedActors[i]->MyModel->GetWorldBBox();
                         };

                         Vector Scale = MaxScale.max - MaxScale.min;

                         boxX->Text=Convert::ToString(Scale.x);
                         boxY->Text=Convert::ToString(Scale.y);
                         boxZ->Text=Convert::ToString(Scale.z);
                     }
                     else
                     {
                         boxX->Text= " ";
                         boxY->Text= " ";
                         boxZ->Text= " ";
                     };

                 }

                 //Engine Logging
                 if (editor->m_Log.size()>0)
                 {
                     for (int i=0;i<editor->m_Log.size();i++)
                         richTextBox1->AppendText( gcnew System::String(editor->m_Log[i].c_str()));

                     int offset = 0; 
                     int Line = richTextBox1->Lines->Length - 4;
                     for (int i = 0; i < Line - 1 && i < richTextBox1->Lines->Length; i++) 
                     { 
                         offset += richTextBox1->Lines[i]->Length + 1; 
                     } 
                     richTextBox1->Select(offset,0);

                     richTextBox1->Focus();
                     richTextBox1->ScrollToCaret();
                     editor->ClearLog();
                 }

                 // Statistics printing
                 static float UpdateTime = 0;
                 // Update 10 times a second
                 if(tabPanelRight->SelectedIndex == 3 && Profiler::Get()->NumFrames*GDeltaTime > AveragingTime)
                 {
                     UpdateTime = GSeconds;

                     System::Text::StringBuilder^ sBuilder= gcnew System::Text::StringBuilder(1000);

                     Profiler* profiler = Profiler::Get();

                     sBuilder->Append("Drawn: ");
                     sBuilder->Append((int)profiler->NumDraws/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("Tris: ");
                     sBuilder->Append((int)Engine::Instance()->RenderSys->TrisPerFrame());
                     sBuilder->AppendLine();

                     sBuilder->Append("Verts: ");
                     sBuilder->Append((int)Engine::Instance()->RenderSys->VertsPerFrame());
                     sBuilder->AppendLine();

                     sBuilder->Append("Dynamic: ");
                     sBuilder->Append((int)Engine::Instance()->RenderSys->DynamicLightsDrawn());
                     sBuilder->AppendLine();

                     sBuilder->Append("FXDraws: ");
                     sBuilder->Append((int)(profiler->FXDraws/profiler->NumFrames));
                     sBuilder->AppendLine();

                     
                     sBuilder->Append("View: x:");
                     sBuilder->Append((int)editor->m_Camera.Location.x);
                     sBuilder->Append(" y:");
                     sBuilder->Append((int)editor->m_Camera.Location.y);
                     sBuilder->Append(" z:");
                     sBuilder->Append((int)editor->m_Camera.Location.z);
                     sBuilder->AppendLine();


                     sBuilder->Append("FPS: ");
                     sBuilder->Append((int)(1000.0f/(profiler->DeltaMS/profiler->NumFrames)));
                     sBuilder->AppendLine();

                     sBuilder->Append("Est FPS: ");
                     sBuilder->Append((int)(1000.0f/(profiler->GetTotalMS()/profiler->NumFrames)));
                     sBuilder->AppendLine();

                     sBuilder->Append("RenderMS: ");
                     sBuilder->Append((int)(profiler->RenderMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("PreRenderMS: ");
                     sBuilder->Append((int)(profiler->RenderPreMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("PostRenderMS: ");
                     sBuilder->Append((int)(profiler->RenderPostMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("BatchMS: ");
                     sBuilder->Append((int)(profiler->BatchMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("EditorMS: ");
                     sBuilder->Append((int)(profiler->EditorMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("ScriptMS: ");
                     sBuilder->Append((int)(profiler->ScriptMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("C# Actors: ");
                     sBuilder->Append((int)(profiler->ScriptActors/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("TickMS: ");
                     sBuilder->Append((int)(profiler->TickMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("PresMS: ");
                     sBuilder->Append((int)(profiler->PresentMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("AudioMS: ");
                     sBuilder->Append((int)(profiler->AudioMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("WaterRenderMS: ");
                     sBuilder->Append((int)(profiler->WaterRenderMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("WaterCPUMS: ");
                     sBuilder->Append((int)(profiler->WaterCPUMS/profiler->NumFrames));
                     sBuilder->AppendLine();

                     sBuilder->Append("-----------");
                     sBuilder->AppendLine();

                     sBuilder->Append("Shader Changes: ");
                     sBuilder->Append((int)profiler->ShaderChanges/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("Technique Changes: ");
                     sBuilder->Append((int)profiler->TechniqueChanges/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("Light Changes: ");
                     sBuilder->Append((int)profiler->LightChanges/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("Material Changes: ");
                     sBuilder->Append((int)profiler->MaterialChanges/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("Skinning Changes: ");
                     sBuilder->Append((int)profiler->SkinningChanges/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("FlipTarget() calls: ");
                     sBuilder->Append((int)profiler->FlipTargets/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("SetHDR() calls: ");
                     sBuilder->Append((int)profiler->SetHDRCalls/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("SortRender() calls: ");
                     sBuilder->Append((int)profiler->SortRenders/profiler->NumFrames);
                     sBuilder->AppendLine();

                     sBuilder->Append("Occlusion Queries: ");
                     sBuilder->Append((int)profiler->OcclusionQueries/profiler->NumFrames);
                     sBuilder->AppendLine();


                     statsText->Text = sBuilder->ToString();
                 }

                 Profiler::Get()->Update();
                 Profiler::Get()->EditorMS += StopMiniTimer();
                 updatingUI = false;

             }

    private: void OnIdle(System::Object^ sender, System::EventArgs^ eva)
             {
             }

    private: System::Void newButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void moveButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 moveButton->Checked	= true;
                 rotateButton->Checked	= false;
                 scaleButton->Checked	= false;
                 editor->m_Gizmo.m_Mode = Gizmo::MODE_TRANSLATE;
                 editor->m_bSelectionChanged=true;
                 UpdateUI();
             }

    private: System::Void rotateButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 moveButton->Checked	= false;
                 rotateButton->Checked	= true;
                 scaleButton->Checked	= false;
                 editor->m_Gizmo.m_Mode = Gizmo::MODE_ROTATE;
                 editor->m_bSelectionChanged=true;
                 UpdateUI();
             }

    private: System::Void scaleButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 moveButton->Checked	= false;
                 rotateButton->Checked	= false;
                 scaleButton->Checked	= true;
                 editor->m_Gizmo.m_Mode = Gizmo::MODE_SCALE;
                 editor->m_bSelectionChanged=true;
                 UpdateUI();
             }

    private: System::Void fileToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void openMenuButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }
    private: System::Void playToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 // Play!
                 this->splitContainer2->Panel1->Focus();
                 editor->UnSelectAll();
                 editor->SetEditorMode(false);
             }
    private: System::Void openButton_Click(System::Object^  sender, System::EventArgs^  e)
             {

             }

    private: System::Void reloadSkyToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->m_World->ReloadSkyController();
             }

             //
             // File->Open Scene
             //
    private: System::Void openToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                 {
                     String^ str = openFileDialog1->FileName;
                     assert(!bInGameDLL);
                     LoadMap(str);			 
                     AddMRU(str->Substring(str->LastIndexOf("\\")+1));
                 }

             }

              /// <summary>
             /// Export selected actor to a file format
             /// </summary>
    private: System::Void exportSelectedToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 exportFileDialog->InitialDirectory = "..\\Models\\";
                 if(exportFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                 {
                     String^ str = exportFileDialog->FileName;
                     if(AsLower(ToCppString(str)).find(".obj") != -1)
                     {
                         // Set prefs first
                         OBJForm->stripHierarchy->Enabled = false;
                         OBJForm->bExport = true;
                         OBJForm->unitScale->Text	= gcnew String(ToStr(OBJSave::s_Scale).c_str());
                         OBJForm->radioD3D->Checked  = (OBJSave::s_CoordSystem == OBJSave::D3D);
                         OBJForm->radioMax->Checked  = (OBJSave::s_CoordSystem == OBJSave::MAX);
                         OBJForm->radioMaya->Checked = (OBJSave::s_CoordSystem == OBJSave::MAYA);

                         OBJForm->ShowDialog();

                         // Now we have our OBJ preferences, set them in the OBJ class
                         OBJSave::s_Scale = Convert::ToDouble(OBJForm->unitScale->Text,Globalization::CultureInfo::CurrentCulture);
                         if(OBJForm->radioD3D->Checked)
                             OBJSave::s_CoordSystem = OBJSave::D3D;
                         if(OBJForm->radioMax->Checked)
                             OBJSave::s_CoordSystem = OBJSave::MAX;
                         if(OBJForm->radioMaya->Checked)
                             OBJSave::s_CoordSystem = OBJSave::MAYA;
                     }

                     editor->SaveSelected(ToCppString(str).c_str());
                 }
             }

             // <summary>
             // File->Import Mesh
             // </summary>
    private: System::Void importMeshToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(importMeshDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                 {
                     String^ str = importMeshDialog->FileName;

                     bool isOBJ = AsLower(ToCppString(str)).find(".obj") != -1;
                     bool isX = AsLower(ToCppString(str)).find(".x") != -1;
                     if(isOBJ)
                     {
                         // Set prefs first
                         OBJForm->stripHierarchy->Enabled = false;
                         OBJForm->bExport = false;
                         OBJForm->unitScale->Text	= gcnew String(ToStr(OBJLoad::s_Scale).c_str());
                         OBJForm->radioD3D->Checked  = (OBJLoad::s_CoordSystem == OBJLoad::D3D);
                         OBJForm->radioMax->Checked  = (OBJLoad::s_CoordSystem == OBJLoad::MAX);
                         OBJForm->radioMaya->Checked = (OBJLoad::s_CoordSystem == OBJLoad::MAYA);

                         OBJForm->ShowDialog();

                         // Now we have our OBJ preferences, set them in the OBJ class
                         OBJLoad::s_Scale = Convert::ToDouble(OBJForm->unitScale->Text,Globalization::CultureInfo::CurrentCulture);
                         if(OBJForm->radioD3D->Checked)
                             OBJLoad::s_CoordSystem = OBJLoad::D3D;
                         if(OBJForm->radioMax->Checked)
                             OBJLoad::s_CoordSystem = OBJLoad::MAX;
                         if(OBJForm->radioMaya->Checked)
                             OBJLoad::s_CoordSystem = OBJLoad::MAYA;
                     }
                     // X-Files
                     else if(isX && AsLower(ToCppString(str)).find(".xml") == -1)
                     {
                         OBJForm->stripHierarchy->Enabled = true;
                         OBJForm->bExport = false;
                         OBJForm->unitScale->Text	= gcnew String(ToStr(XFileLoad::s_Scale).c_str());
                         OBJForm->radioD3D->Enabled  = false;
                         OBJForm->radioMax->Enabled  = false;
                         OBJForm->radioMaya->Enabled = false;

                         OBJForm->ShowDialog();

                         XFileLoad::s_Scale = Convert::ToDouble(OBJForm->unitScale->Text,Globalization::CultureInfo::CurrentCulture);
                         XFileLoad::s_StripHierarchy = OBJForm->stripHierarchy->Checked;
                         OBJForm->radioD3D->Enabled  = true;
                         OBJForm->radioMax->Enabled  = true;
                         OBJForm->radioMaya->Enabled = true;
                     }


                     XFileLoad::s_UseSkinning = false; // No skinning on RB imported models

                     assert(!bInGameDLL);
                     Prefab* prefab = editor->AddPrefab(ToCppString(str));
                     if(prefab)
                         prefab->bExportable = true;

                     if(prefab && isOBJ)
                     {
                        // Center pivot
                        Matrix tm;
                        MeshOps::Convert(prefab->MyModel->m_pFrameRoot->GetMesh(),MeshOps::CenterPivot,tm);
                        prefab->Location = tm.m3;
                     }
                     else if(prefab)
                     {
                         prefab->Location = editor->m_Camera.Location + editor->m_Camera.Direction*15;
                     }

                     XFileLoad::s_UseSkinning = true;
                 }
             }

    private: System::Void saveButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->SaveScene();
             }
             // <summary>
             // File->Save
             // </summary>
    private: System::Void saveMenuButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->SaveScene();
             }
             /// <summary>
             /// File->Save As
             /// </summary>
    private: System::Void saveAsMenuButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 saveFileDialog->InitialDirectory = "..\\Maps\\";
                 if(saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                 {
                     String^ str = saveFileDialog->FileName;
                     editor->SaveScene(ToCppString(str).c_str());
                     editor->m_World->m_FileName = ToCppString(str);

                     AddMRU(str->Substring(str->LastIndexOf("\\")+1));
                 }
             }

             /// <summary>
             /// Edit->Delete
             /// </summary>
    private: System::Void deleteToolStripMenuItem_Click_1(System::Object^  sender, System::EventArgs^  e)
             {
                 for(int i=0;i<selectForm->selectObjectsTable->Rows->Count;i++)
                 {
                     string name = ToCppString(selectForm->selectObjectsTable->Rows->Item[i]->Cells[0]->Value->ToString());
                     for(int j=0;j<editor->m_SelectedActors.size();j++)
                     {
                         if(editor->m_SelectedActors[j]->m_Name == name){
                             selectForm->selectObjectsTable->Rows->RemoveAt(i);
                             i--;
                             break;
                         }
                     }
                 }

                 editor->DeleteSelected();
                 UpdateSelectedList();
             }

    private: System::Void buildToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }


    private: System::Void buildSelectedFastToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 callback.bCancel = false;
                 editor->Compile(editor->m_SelectedActors,true, (CompilerCallback*)&callback);
             }

    private: System::Void buildFastToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 callback.bCancel = false;
                 editor->CompileScene(true, (CompilerCallback*)&callback);
             }

    private: System::Void buildSelectedToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 callback.bCancel = false;
                 editor->Compile(editor->m_SelectedActors,false,(CompilerCallback*)&callback);
             }

             /// <summary>
             /// Compiles Scene
             /// </summary>
    private: System::Void buildAllToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->CompileScene(false, (CompilerCallback*)&callback);
             }

             /// <summary>
             /// Adds a Light
             /// </sum
    private: System::Void toolStripButton1_Click(System::Object^  sender, System::EventArgs^  e)
             { 
                 Light* light = new Light(editor->m_World);
                 light->IgnoreKeyframes();
                 light->Location = editor->m_Camera.Location + editor->m_Camera.Direction*15;
                 light->bExportable = true;

                 // Generate new light name
                 light->m_Name = "Light";
                 editor->FixName(light);

                 editor->UnSelectAll();
                 editor->SelectActor(light);
             }



             /// <summary>
             /// Changes an Actor
             /// </sum
    private: System::Void changeActorButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 try
                 {
                     vector<Actor*> actors = editor->m_SelectedActors;
                     for(int i=0;i<actors.size();i++)
                     {
						 Actor* changedActor = actors[i];

                         Actor* actor=Factory::create(ToCppString(actorsCombo->SelectedItem->ToString()),editor->m_World);
                         actor->Location = editor->m_Camera.Location + editor->m_Camera.Direction*15;
                         actor->bExportable = true;

                         // Generate new actor name
                         actor->m_Name = ToCppString(actorsCombo->SelectedItem->ToString());
                         editor->FixName(actor);
                         editor->UnSelectAll();
                         editor->SelectActor(actor);

                         // Copy over core data
                         actor->MyModel  = changedActor->MyModel;
                         actor->Location = changedActor->Location;
                         actor->Rotation = changedActor->Rotation;
                         actor->MyModel  = changedActor->MyModel;

                         // Delete old actor
                         changedActor->MyModel = NULL; // So it doesn't get deleted
						 changedActor->MyWorld->RemoveActor(changedActor);
                         delete changedActor;
                     }
                 }
                 catch (Exception ^)
                 {
                     System::Windows::Forms::MessageBox::Show("Can't Add Actor");
                 }
             }

             /// <summary>
             /// Adds an Actor
             /// </sum
    private: System::Void addActorButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 try
                 {
                     Actor* actor=Factory::create(ToCppString(actorsCombo->SelectedItem->ToString()),editor->m_World);
                     actor->Location = editor->m_Camera.Location + editor->m_Camera.Direction*15;
                     actor->bExportable = true;

                     // Generate new actor name
                     actor->m_Name = ToCppString(actorsCombo->SelectedItem->ToString());
                     editor->FixName(actor);
                     editor->UnSelectAll();
                     editor->SelectActor(actor);
                     editor->m_World->RegenerateOcclusionTree();
                 }
                 catch (Exception ^)
                 {
                     System::Windows::Forms::MessageBox::Show("Can't Add Actor");
                 }
             }

    private: System::Void exitMenuButton_Click_1(System::Object^  sender, System::EventArgs^  e)
             {
                 this->Close();
             }

    private: System::Void slowToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 slowToolStripButton->Checked	= true;
                 mediumToolStripButton->Checked = false;
                 fastToolStripButton->Checked	= false;
                 editor->SetCamSpeed(10.0f);
             }

    private: System::Void mediumToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 slowToolStripButton->Checked	= false;
                 mediumToolStripButton->Checked = true;
                 fastToolStripButton->Checked	= false;

                 editor->SetCamSpeed(50.0f);
             }

    private: System::Void fastToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 slowToolStripButton->Checked	= false;
                 mediumToolStripButton->Checked = false;
                 fastToolStripButton->Checked	= true;
                 editor->SetCamSpeed(200.0f);
             }

    private: System::Void copyToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->CloneSelected();
             }

    private: System::Void propertyEditor_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }



    private: System::Void splitContainer1_Panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }

    private: System::Void splitContainer1_Panel2_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }

    private: System::Void xToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void toolStripTextBox3_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void redoButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void splitContainer1_Panel2_Paint_1(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }
    private: System::Void statusStripPanel1_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void tabPage2_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }

    private: System::Void buttonAddBlockers_Click(System::Object^ sender, System::EventArgs^ e)
             {
                 // Get selected names
                 vector<string> blockers;
                 for(int i=0;i<listBoxScene->SelectedItems->Count;i++)
                 {
                     std::string selectedName=ToCppString(listBoxScene->SelectedItems[i]->ToString());
                     if(selectedName.size() < 1)continue;

                     //groups
                     if(selectedName.c_str()[0]=='{')
                     { 
                         std::vector<std::string> NamesToAdd = 
                             editor->GetModelFrameNamesFromGroup((char *)selectedName.c_str());
                         for(int j=0;j<NamesToAdd.size();j++)
                             blockers.push_back(NamesToAdd.at(j)); 
                     }
                     //single instances
                     else
                     { 
                         blockers.push_back(ToCppString(listBoxScene->SelectedItems[i]->ToString()));
                     };
                 }
                 for(int j=0;j<(*selectedMeshes).size();j++)
                 {
                     // Add item to blocker list. Check for uniqueness
                     (*selectedMeshes)[j]->GetMesh()->m_SHOptions.AddInBlockers(blockers);
                 }
                 // Selection changed, update UI
                 UpdatePropTabs();
             }


    private: System::Void buttonRemoveBlockers_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 // Get selected names
                 vector<string> blockers;
                 for(int i=0;i<listBoxBlockers->SelectedItems->Count;i++)
                 {
                     blockers.push_back(ToCppString(listBoxBlockers->SelectedItems[i]->ToString()));
                 }

                 for(int j=0;j<(*selectedMeshes).size();j++)
                 {
                     // Add item to blocker list. Check for uniqueness
                     (*selectedMeshes)[j]->GetMesh()->m_SHOptions.RemoveInBlockers(blockers);
                 }

                 // Selection changed, update UI
                 UpdatePropTabs();
             }

             /// <summary>
             /// Refresh button
             /// </summary>
    private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 Mesh* m = (*selectedMeshes)[0]->GetMesh();

                 // Update material
                 int index = 0;
                 if(comboSubmesh->SelectedIndex != -1)
                     index = comboSubmesh->SelectedIndex;
                 if(m->m_Materials.size() > index)
                 {
                     m->m_Materials[index]->Initialize(m->m_Materials[index]->m_Shader->GetFilename().c_str(),m->m_Materials[index]->m_Token.c_str(),true);
                 }
             }
    private: System::Void gameToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }


    private: System::Void undoToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->PopUndo();
             }

    private: System::Void selectObjectsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 selectForm->Fill();
                 selectForm->Show();
             }

    private: System::Void toolStripSeparator3_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void takeScreenshot_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 RenderDevice::Instance()->TakeScreenshot();
             }

    private: System::Void buttonEditMat_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

             /// <summary>
             /// New submesh selected
             /// </summary>
    private: System::Void comboSubmesh_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 UpdateRenderProps();
                 AssetBrowser::me->SetSubMeshIndex(comboSubmesh->SelectedIndex);
             }


    private: System::Void toolsToolbar_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

             // <summary>
             // Merge in a new world or model
             // </summary>
    private: System::Void mergeToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                 {
                     String^ str = openFileDialog1->FileName;
                     editor->MergeScene(ToCppString(str).c_str());
                     editor->m_World->RegenerateOcclusionTree();
                 }
             }

             // <summary>
             // File->Save Selected...
             // </summary>
    private: System::Void saveSelectedToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 saveFileDialog->InitialDirectory = "..\\Maps\\";
                 if(saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                 {
                     String^ str = saveFileDialog->FileName;

                     // Backup world actors
                     vector<Actor*> actors = editor->m_World->m_Actors;
                     // Fill with selected
                     editor->m_World->m_Actors = editor->m_SelectedActors;
                     editor->SaveScene(ToCppString(str).c_str());
                     // Put back world actors
                     editor->m_World->m_Actors = actors;
                 }
             }

             // <summary>
             // Select Object(s) toolbar
             // </summary>
    private: System::Void toolStripButton2_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 selectForm->Fill();
                 selectForm->Show();
             }

    private: System::Void toolStripButton4_Click_1(System::Object^  sender, System::EventArgs^  e)
             {
                 if(editor->m_SelectedActors.empty())	return;

                 BBox box;

                 for(int i = 0; i < editor->m_SelectedActors.size(); i++)
                 { 
                     if(editor->m_SelectedActors[i]->MyModel)
                         box += editor->m_SelectedActors[i]->MyModel->GetWorldBBox();
                 }

                 Vector BoxMid = (box.min + box.max)/2;
                 float SpereRad = (float) (BoxMid - box.max).Length();

                 float ExtraDistance=2.5f;

                 Vector NewPosition = (BoxMid + 
                     (-editor->m_Camera.Direction*(SpereRad*ExtraDistance)));

                 editor->m_Camera.Location = NewPosition;
             }

    private: System::Void orbitViewToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(editor->m_SelectedActors.empty())
                 {
                     editor->m_ViewMode=0;
                     orbitViewToolStripButton->Checked=false;
                     return;
                 }


                 if(orbitViewToolStripButton->Checked)
                 {
                     editor->m_ViewMode=EDITOR_ORBIT_VIEW;
                     if(!editor->m_SelectedActors.empty())
                     {
                         BBox box;

                         for(int i = 0; i < editor->m_SelectedActors.size(); i++)
                         { 
                             if(editor->m_SelectedActors[i]->MyModel)
                                 box += editor->m_SelectedActors[i]->MyModel->GetWorldBBox();
                         }

                         Vector BoxMid = (box.min + box.max)/2;
                         float SpereRad = (float) (BoxMid - box.max).Length();
                         float Distance = (float) (editor->m_Camera.Location - BoxMid).Length();

                         Vector NewPosition = (BoxMid + (-editor->m_Camera.Direction*(Distance)));

                         editor->m_Camera.Location = NewPosition;
                     };
                 }
                 else
                 {
                     editor->m_ViewMode=0;	
                     return;
                 };


             }

    private: System::Void toolStripAssetBrowser_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 assetBrowser->Show();
             }

    private: System::Void editToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void newMenuButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->UnSelectAll();
                 ResetActorsIDs();
                 ClearSelectedLists();
                 editor->m_World->NewWorld();
             }

    private: System::Void boxX_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void insertToolBarSep_Click(System::Object^  sender, System::EventArgs^  e)
             {

                 editor->UnSelectAll();
             }

             /// <summary>
             /// Edit world properties - fills propertygrid with world EditorVars
             /// </summary>
    private: System::Void editWorldPropertiesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 // Don't want to display actor params, we're listing world params...
                 editor->UnSelectAll(); 
                 propHelper->UpdateVars(editor->m_World->m_EditorVars);
                 propertyGrid1->SelectedObject = propHelper->class1;
             }


    private: System::Void splitContainer2_Panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }

    private: System::Void hideSelectedToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->HideActors(editor->m_SelectedActors);
                 /*
                 for(int i = 0; i < editor->m_SelectedActors.size(); i++)
                 { 
                 if(editor->m_SelectedActors[i])
                 editor->m_SelectedActors[i]->IsHidden=true;
                 }*/

                 editor->UnSelectAll(); 
             }

    private: System::Void freezeSelectedToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->FreezeActors(editor->m_SelectedActors);

                 /*
                 for(int i = 0; i < editor->m_SelectedActors.size(); i++)
                 { 
                 if(editor->m_SelectedActors[i])
                 editor->m_SelectedActors[i]->IsFrozen=true;
                 }*/

                 editor->UnSelectAll(); 
             }

    private: System::Void unHideAllToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {

                 editor->UnHideActors(editor->m_World->m_Actors);

                 //for(int i=0;i<editor->m_World->m_Actors.size();i++)
                 //	 editor->m_World->m_Actors[i]->IsHidden=false;

                 editor->UnSelectAll(); 
             }

    private: System::Void unFreezeAllToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->UnFreezeActors(editor->m_World->m_Actors);

                 //for(int i=0;i<editor->m_World->m_Actors.size();i++)
                 //	 editor->m_World->m_Actors[i]->IsFrozen=false;

                 editor->UnSelectAll(); 
             }

    private: System::Void hideUnselectedToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 for(int i=0;i<editor->m_World->m_Actors.size();i++)
                 {
                     if(editor->m_World->m_Actors[i]->IsLight())
                         continue; // Never hide lights, becasue ppl still want to see their meshes when they hit hide unselected

                     bool found = false;
                     for(int j=0;j<editor->m_SelectedActors.size();j++)
                     {
                         if(editor->m_SelectedActors[j] == editor->m_World->m_Actors[i])
                         {
                             found = true;
                             break;
                         }
                     }
                     if(!found)
                         editor->m_World->m_Actors[i]->IsHidden = true;
                 }
             }

    private: System::Void helpToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void visitOnlineHelpToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 System::Diagnostics::Process::Start("IExplore.exe","http://reality.artificialstudios.com");
             }

    private: System::Void aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 System::Windows::Forms::MessageBox::Show("Reality Builder (c) Artificial Studios 2001-2004\nhttp://www.artificialstudios.com","About");
             }

    private: System::Void mainMenu_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void sTGEditBoxToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void sTAToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {

             }

    private: System::Void sTGToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->m_bSnapToGrid=sTGToolStripButton->Checked;
             }

    private: System::Void hideUnSelectedToolStripMenuItem1_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void contextMenuStrip1_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void toolStrip3_Layout(System::Object^  sender, System::Windows::Forms::LayoutEventArgs^  e)
             {
             }

    private: System::Void toolStrip3_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
             {
             }

    private: System::Void splitContainer2_Panel1_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
             {
                 Vector ActorsLocation = Vector(0,0,0);
                 if(editor->m_SelectedActors.size()>0 && editor->m_bDragging)
                 {	
                     if(moveButton->Checked)
                     {
                         for(int i=0;i<editor->m_SelectedActors.size();i++)				
                             ActorsLocation+= (editor->m_SelectedActors.at(0)->Location / 
                             editor->m_SelectedActors.size());				 				 

                         boxX->Text=Convert::ToString(ActorsLocation.x);
                         boxY->Text=Convert::ToString(ActorsLocation.y);
                         boxZ->Text=Convert::ToString(ActorsLocation.z);
                     }
                     else if(rotateButton->Checked && editor->m_SelectedActors.size()==1)
                     {
                         float AngX = 0, AngY = 0, AngZ = 0;

                         editor->GrabAngles(AngX, AngY, AngZ);

                         boxX->Text=Convert::ToString(AngX*PICONVERSION);
                         boxY->Text=Convert::ToString(AngY*PICONVERSION);
                         boxZ->Text=Convert::ToString(AngZ*PICONVERSION);
                     } else if(scaleButton->Checked && editor->m_SelectedActors.size()>0)
                     {	
                         BBox MaxScale;
                         for(int i=0;i<editor->m_SelectedActors.size();i++)
                         {
                             if(editor->m_SelectedActors[i]->MyModel)
                                 MaxScale += editor->m_SelectedActors[i]->MyModel->GetWorldBBox();
                         };

                         Vector Scale = MaxScale.max - MaxScale.min;

                         boxX->Text=Convert::ToString(Scale.x);
                         boxY->Text=Convert::ToString(Scale.y);
                         boxZ->Text=Convert::ToString(Scale.z);
                     }
                     else
                     {
                         boxX->Text= " ";
                         boxY->Text= " ";
                         boxZ->Text= " ";
                     };
                 }			 
             }

    private: System::Void modifiersToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void boxY_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void cToolStripButton1_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void angleBoxX_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void angleBoxX_Leave(System::Object^  sender, System::EventArgs^  e)
             {

             }

    private: System::Void angleBoxY_Leave(System::Object^  sender, System::EventArgs^  e)
             {

             }

    private: System::Void angleBoxZ_Leave(System::Object^  sender, System::EventArgs^  e)
             {

             }
             /// Exclude/Include ADD
    private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 // Add new items to light
                 for(int j=0;j<editor->m_SelectedActors.size();j++)
                 {
                     if(!editor->m_SelectedActors[j]->m_HasIncludeExcludeList)
                         continue;

                     Actor * actor=editor->m_SelectedActors[j];
                     for(int i=0;i<listSceneMeshes1->SelectedItems->Count;i++)
                     {
                         std::string actorName=ToCppString(listSceneMeshes1->SelectedItems[i]->ToString());

                         if(actorName.size() < 1)continue;
                         //groups
                         if(actorName.c_str()[0]=='{')
                         {							 
                             std::vector<std::string> NamesToAdd = 
                                 editor->GetActorsFromGroup((char *)actorName.c_str());

                             for(int j=0;j<NamesToAdd.size();j++)
                             {
                                 bool found = false;
                                 for(int k=0;k<actor->m_ExcludeList.size();k++)
                                 {
                                     if (actor->m_ExcludeList[k] == NamesToAdd.at(j))
                                     {
                                         found =true;
                                         continue;
                                     }
                                 }
                                 if (!found) actor->m_ExcludeList.push_back(NamesToAdd.at(j).c_str());
                             };
                         }
                         //directNames
                         else
                         {

                             bool found = false;
                             for(int k=0;k<actor->m_ExcludeList.size();k++)
                             {
                                 if (actor->m_ExcludeList[k]== actorName)
                                 {
                                     found =true;
                                     continue;
                                 }
                             }

                             if (!found) actor->m_ExcludeList.push_back(actorName);
                         };
                     }

                 }
                 UpdatePropTabs();
             }

             /// Exclude/Include REMOVE
    private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 for(int k=0;k<editor->m_SelectedActors.size();k++)
                 {
                     if(!editor->m_SelectedActors[k]->m_HasIncludeExcludeList)
                         continue;

                     Actor * actor=editor->m_SelectedActors[k];
                     for(int i=0;i<listExclusion->SelectedItems->Count;i++)
                     {
                         for(int j=0;j<actor->m_ExcludeList.size();j++)
                         {
                             if(actor->m_ExcludeList[j] ==ToCppString(listExclusion->SelectedItems[i]->ToString()))
                             {
                                 actor->m_ExcludeList.erase(actor->m_ExcludeList.begin()+j);
                                 break;
                             }
                         }
                     }

                 }
                 UpdatePropTabs();
             }

    private: System::Void checkIsExculsion_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 for(int i=0;i<editor->m_SelectedActors.size();i++)
                     editor->m_SelectedActors[i]->m_IsExcludeList = checkIsExculsion->Checked;
             }

    private: System::Void splitContainer2_Panel1_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
             {
             }

    private: System::Void toolStripComboBox1_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

             /// <summary>
             /// View perspective changing
             /// </summary>
    private: System::Void toolStripComboBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 // Calculate Distance
                 BBox box;
                 if (editor->m_SelectedActors.size())
                 {
                     for(int i = 0; i < editor->m_SelectedActors.size(); i++)
                     { 
                         if(editor->m_SelectedActors[i]->MyModel)
                             box += editor->m_SelectedActors[i]->MyModel->GetWorldBBox();
                     }
                 }
                 else
                 {
                     for(int i = 0; i < editor->m_World->m_Actors.size(); i++)
                     { 
                         if(editor->m_World->m_Actors[i]->MyModel)
                             box += editor->m_World->m_Actors[i]->MyModel->GetWorldBBox();
                     }
                 }
                 float aspRatio =(float) RenderDevice::Instance()->GetViewportX()/ RenderDevice::Instance()->GetViewportY();
                 Vector BoxMid = (box.min + box.max)/2;
                 float SpereRad = (float) (BoxMid - box.max).Length();
                 float distance =SpereRad*2;
                 int selectedIndex=toolStripComboBox1->SelectedIndex;
                 float maxSide=abs(max(box.max.x,box.max.y));
                 maxSide=max(maxSide,box.max.z) * 1.5f;
                 editor->m_ScreenHeight=maxSide;
                 D3DXMATRIX matProj;
                 Matrix camTransform;

                 // Store old perspective
                 static Camera oldCam;
                 if(selectedIndex != 0 && editor->m_CurrentView == PERSPECTIVE_VIEW)
                     oldCam = editor->m_Camera;

                 switch (selectedIndex)
                 {
                 case 0://Perspective
                     if(editor->m_CurrentView != PERSPECTIVE_VIEW) // Never set cam unless it changed, or will be empty
                     {
                         editor->m_CurrentView = PERSPECTIVE_VIEW;
                         editor->m_Camera = oldCam;
                     }
                     break;
                 case 1://Left
                     editor->m_Zooming = 1;
                     editor->m_CurrentView = LEFT_VIEW;
#ifndef _NOD3DX
                     D3DXMatrixOrthoLH(&matProj,maxSide* aspRatio,maxSide ,1,10000);
#endif
                     editor->m_Camera.Direction.Set(1,0,0);
                     editor->m_Camera.Location=BoxMid;
                     editor->m_Camera.Location.x+=-distance;
                     editor->m_Camera.projection= *(Matrix*)&matProj;
                     editor->m_Camera.Update();
                     break;
                 case 2://Top
                     editor->m_Zooming = 1;
                     editor->m_CurrentView = TOP_VIEW;
#ifndef _NOD3DX
                     D3DXMatrixOrthoLH(&matProj,maxSide* aspRatio,maxSide ,1,10000);
#endif
                     editor->m_Camera.Direction.Set(0,-1,0);
                     editor->m_Camera.Location=BoxMid;
                     editor->m_Camera.Location.y+=distance;
                     editor->m_Camera.projection= *(Matrix*)&matProj;
                     break;
                 case 3://Right
                     editor->m_Zooming = 1;
                     editor->m_CurrentView = RIGHT_VIEW;
#ifndef _NOD3DX
                     D3DXMatrixOrthoLH(&matProj,maxSide * aspRatio,maxSide,1,10000);
#endif
                     editor->m_Camera.Direction.Set(-1,0,0);
                     editor->m_Camera.Location=BoxMid;
                     editor->m_Camera.Location.x+=distance;
                     editor->m_Camera.projection= *(Matrix*)&matProj;
                     editor->m_Camera.Update();
                     break;
                 case 4://Bottom
                     editor->m_Zooming = 1;
                     editor->m_CurrentView = BOTTOM_VIEW;
#ifndef _NOD3DX
                     D3DXMatrixOrthoLH(&matProj,maxSide* aspRatio,maxSide ,1,10000);
#endif
                     editor->m_Camera.Direction.Set(0,1,0);
                     editor->m_Camera.Location=BoxMid;
                     editor->m_Camera.Location.y-=distance;
                     editor->m_Camera.projection= *(Matrix*)&matProj;
                     break;
                 case 5://Front
                     editor->m_Zooming = 1;
                     editor->m_CurrentView = FRONT_VIEW;
#ifndef _NOD3DX
                     D3DXMatrixOrthoLH(&matProj,maxSide * aspRatio,maxSide,1,10000);
#endif
                     editor->m_Camera.Direction.Set(0,0,1);
                     editor->m_Camera.Location=BoxMid;
                     editor->m_Camera.Location.z-=distance;
                     editor->m_Camera.projection= *(Matrix*)&matProj;
                     editor->m_Camera.Update();
                     break;
                 case 6://Back
                     editor->m_Zooming = 1;
                     editor->m_CurrentView =BACK_VIEW;
#ifndef _NOD3DX
                     D3DXMatrixOrthoLH(&matProj,maxSide* aspRatio,maxSide ,1,10000);
#endif
                     editor->m_Camera.Direction.Set(0,0,-1);
                     editor->m_Camera.Location=BoxMid;
                     editor->m_Camera.Location.z+=distance;
                     editor->m_Camera.projection= *(Matrix*)&matProj;
                     break;
                 }
             }

    private: System::Void linkLabel1_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e)
             {
                 if (linkLabel1->Text == "Hide")
                 {
                     linkLabel1->Text = "Show";
                     panel2->Height=panel3->Height;
                 }
                 else
                 {
                     linkLabel1->Text = "Hide";
                     panel2->Height=98;
                 }
             }

             /// <summary>
             /// Wireframe toggle
             /// </summary>
    private: System::Void wireframeToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->m_bWireFrameMode = !editor->m_bWireFrameMode;
             }

    private: System::Void bboxToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->m_World->m_RenderOcclusionBoxes = !editor->m_World->m_RenderOcclusionBoxes ;
             }

    private: System::Void occToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->m_World->m_UseOcclusionCulling = occToolStripMenuItem->Checked;
             }

    private: System::Void ClearSelectedLists()
             {
                 selectedListComboBox->Items->Clear();
                 selectedListBox->Items->Clear();
                 for(int i=0;i<editor->m_ActorLists.size();i++)
                     editor->m_ActorLists.at(i).m_SelectedActors.clear();
             };

    private: System::Void UpdateSelectedList()
             {				 
                 int selectedIndex = selectedListComboBox->SelectedIndex;
                 if(selectedIndex<0) return;
                 selectedListBox->Items->Clear();

                 for(int i=0;i<editor->m_ActorLists.at(selectedIndex).m_SelectedActors.size();i++)
                     selectedListBox->Items->Add( gcnew String(
                     editor->m_ActorLists.at(selectedIndex).m_SelectedActors.at(i)->m_Name.c_str()));

                 //selectedListComboBox->SelectedIndex=selectedIndex;
             };

    private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 int selectedIndex = selectedListComboBox->SelectedIndex;
                 if(selectedIndex<0)return;

                 selectedListComboBox->Items->RemoveAt(selectedIndex);	

                 editor->m_ActorLists.at(selectedIndex).m_SelectedActors.clear();
                 editor->m_ActorLists.erase(editor->m_ActorLists.begin() + selectedIndex);

                 FillSelectionLists();
             }

    private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 static int numLists=0;
                 try
                 {
                     editor->m_ActorLists.push_back(ActorSelectedList(numLists));
                     selectedListComboBox->Items->Add(gcnew String(editor->m_ActorLists.back().ListName.c_str()));
                     selectedListComboBox->SelectedIndex=selectedListComboBox->Items->Count-1;

                     numLists++;

                     //If you set this here you lose what was selected in selectedindex
                     //FillSelectionLists();					 
                 }
                 catch (Exception ^ ex)
                 {
                 }
             }

             //Selected to List
    private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e)
             {			
                 int selectedIndex = selectedListComboBox->SelectedIndex;

                 if(selectedIndex<0)return;

                 //editor->m_ActorLists.at(selectedIndex).m_SelectedActors.clear();

                 for(int i=0;i<editor->m_SelectedActors.size();i++)
                 {
                     bool found = false;
                     for(int k = 0; k < editor->m_ActorLists.at(selectedIndex).m_SelectedActors.size(); k++)
                     {
                         if(editor->m_ActorLists.at(selectedIndex).m_SelectedActors[k] == editor->m_SelectedActors[i])
                             found = true;
                     }
                     if(!found)
                     {
                         editor->m_ActorLists.at(selectedIndex).m_SelectedActors.push_back( editor->m_SelectedActors.at(i) );
                     }
                 }

                 UpdateSelectedList();	
             }

             //List to Selected
    private: System::Void button7_Click(System::Object^  sender, System::EventArgs^  e)
             {			
                 int selectedIndex = selectedListComboBox->SelectedIndex;
                 if(selectedIndex<0)return;

                 editor->UnSelectAll();

                 for(int i=0;i<editor->m_ActorLists.at(selectedIndex).m_SelectedActors.size();i++)
                 {
                     editor->SelectActor(editor->m_ActorLists.at(selectedIndex).m_SelectedActors.at(i));
                 }
             }

    private: System::Void selectedListComboBox_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
             {	
                 curSelectedIndex=selectedListComboBox->SelectedIndex;
                 UpdateSelectedList();
             }

    private: System::Void selectedListComboBox_TextUpdate(System::Object^  sender, System::EventArgs^  e)
             {


             }

    private: System::Void button9_Click(System::Object^  sender, System::EventArgs^  e)
             {				 
                 if(curSelectedIndex<0)return;
                 string newName=ToCppString( selectedListComboBox->Text);

                 bool found=false;
                 for (int i=0;i<editor->m_ActorLists.size();i++)
                 {
                     if (editor->m_ActorLists[i].ListName == newName)
                     {
                         found = true;
                         break;
                     }
                 }
                 if (newName == "")
                 {
                     System::Windows::Forms::MessageBox::Show("Name can't be null.");
                     selectedListComboBox->Text=gcnew String(editor->m_ActorLists.at(curSelectedIndex).ListName.c_str());
                 }
                 else if (found)
                 {
                     System::Windows::Forms::MessageBox::Show("Name is already taken.");
                     selectedListComboBox->Text=gcnew String(editor->m_ActorLists.at(curSelectedIndex).ListName.c_str());
                 }
                 else
                     editor->m_ActorLists.at(curSelectedIndex).ListName = ToCppString( selectedListComboBox->Text);

                 FillSelectionLists();
                 try
                 {
                     selectedListComboBox->SelectedIndex=curSelectedIndex;
                 }
                 catch (Exception^ ex){};
             }
           

    private: System::Void ResetActorsIDs()
             {
				 int LargestID = 0;
                 int NumOfActors = editor->m_World->m_Actors.size();
                 for(int i=0; i<NumOfActors;i++)
				 {
                   if(editor->m_World->m_Actors.at(i)->m_ActorID > LargestID)
					   LargestID = editor->m_World->m_Actors.at(i)->m_ActorID;
				 }
                 ACTOR_ID_COUNT=LargestID+1;
             };


    private: System::Void clearListButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 int selectedIndex = selectedListComboBox->SelectedIndex;
                 if(selectedIndex<0)return;
                 editor->m_ActorLists.at(selectedIndex).m_SelectedActors.clear();
                 UpdateSelectedList();	
             }

    private: System::Void panel4_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }

    private: System::Void selectedListBox_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
             {
             }

             /// <summary>
             /// Regenerate mesh settings
             /// </summary>
    private: System::Void regenerateTangentsNormalsLightingToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 meshSettings->ShowDialog();

                 for(int j=0;j<editor->m_SelectedActors.size();j++)
                 {
                     Actor* actor = editor->m_SelectedActors[j];
                     if(!actor || !actor->MyModel)
                         return;

                     vector<ModelFrame*> frames;
                     actor->MyModel->m_pFrameRoot->EnumerateMeshes(frames);

                     for(int i=0;i<frames.size();i++)
                     {
                         MeshOps::Mend(frames[i]->GetMesh(0),true,
                             Decimal::ToDouble(meshSettings->minCosCrease->Value),
                             Decimal::ToDouble(meshSettings->normalWeighting->Value),
                             meshSettings->fixCylindrical->Checked,
                             meshSettings->respectSplits->Checked);

                         // Regenerate LODs by setting mesh again
                         frames[i]->SetMesh(frames[i]->GetMesh(0));
                     }
                 }

                 
             }

    private: System::Void flipNormalsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->MeshOpSelected(MeshOps::InvertNormals);
             }
    private: System::Void invertMeshToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->MeshOpSelected(MeshOps::Invert);
             }

    private: System::Void redoToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void redoToolStripMenuItem_Click_1(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->PopRedo();
             }

             /// <summary>
             /// Exports a prefab, saving a thumbnail
             /// </summary>
             void SaveSelected(String^ str)
             {
                 string fileNameOut= ToCppString(str);
                 fileNameOut = fileNameOut.substr(0,fileNameOut.find_last_of("."));

                 //Store Cam Position
                 Vector storeCam = editor->m_Camera.Location;
                 // Snap Camera to Selected
                 toolStripButton4_Click_1(nullptr, nullptr);  


                 // Store What actors were hidden and hide all
                 vector<bool> HiddenState;
                 for(int i=0;i<editor->m_World->m_Actors.size();i++)
                 {
                     HiddenState.push_back(editor->m_World->m_Actors.at(i)->IsHidden);
                     editor->m_World->m_Actors.at(i)->IsHidden=true;
                 };

				 vector<Actor *> sActors;
                 // unhide selected
                 for(int i=0;i<editor->m_SelectedActors.size();i++)
				 {
					 sActors.push_back(editor->m_SelectedActors.at(i));
                     editor->m_SelectedActors.at(i)->IsHidden=false;
				 };

				 for(int i=0;i<sActors.size();i++)
					editor->UnSelectActor(sActors.at(i));

                 editor->m_Camera.Update();

                 //				 
                 if(SkyController::Instance)
                     SkyController::Instance->bRender = false;
                 editor->m_RenderDevice->RenderCallback();
                 // needs to be modified
                 editor->m_RenderDevice->SaveThumbnail(fileNameOut.c_str());

				 for(int i=0;i<sActors.size();i++)
					editor->SelectActor(sActors.at(i));

                 if(SkyController::Instance)
                     SkyController::Instance->bRender = true;

                 editor->m_Camera.Location = storeCam;

                 for(int i=0;i<HiddenState.size();i++)		
                     editor->m_World->m_Actors.at(i)->IsHidden=HiddenState.at(i);
                 HiddenState.clear();

                 // Thumbnail stored now... back to normal saving
                 // Backup world actors
                 vector<Actor*> actors = editor->m_World->m_Actors;
                 // Fill with selected
                 editor->m_World->m_Actors = editor->m_SelectedActors;
                 editor->SaveScene(ToCppString(str).c_str());
                 // Put back world actors
                 editor->m_World->m_Actors = actors;

                 assetBrowser->ResetPrefabTree();
             }

    private: System::Void exportSelectedAsPrefabsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 saveFileDialog->InitialDirectory = "..\\Models\\";
                 if(saveFileDialog->ShowDialog() == 
                     System::Windows::Forms::DialogResult::OK)
                 {
                     String^ str = saveFileDialog->FileName;
                     SaveSelected(str);
                 }
             }

    private: System::Void centerPivotToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 editor->MeshOpSelected(MeshOps::CenterPivot);
             }

private: System::Void combineSelectedPrefabsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
         {
             editor->CombineSelected();
         }

         /// <summary>
         /// Split mesh dialog
         /// </summary>
private: System::Void splitMeshToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
         {
             splitForm->ShowDialog();

             int x = Decimal::ToInt32(splitForm->xSegs->Value);
             int y = Decimal::ToInt32(splitForm->ySegs->Value);
             int z = Decimal::ToInt32(splitForm->zSegs->Value);

             for(int j=0;j<editor->m_SelectedActors.size();j++)
             {
                 Actor* actor = editor->m_SelectedActors[j];
                 if(!actor || !actor->MyModel)
                     continue;

                 vector<ModelFrame*> frames;
                 actor->MyModel->m_pFrameRoot->EnumerateMeshes(frames);

                 for(int i=0;i<frames.size();i++)
                 {
                     Matrix tm;
                     MeshOps::Split(frames[i]->GetMesh(0),x,y,z);

                     // Regenerate LODs by setting mesh again
                     frames[i]->SetMesh(frames[i]->GetMesh(0));
                 }
             }
         }

private: System::Void bakePRTLightingToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
         {
             for(int j=0;j<editor->m_SelectedActors.size();j++)
             {
                 Actor* actor = editor->m_SelectedActors[j];
                 if(!actor || !actor->MyModel)
                     continue;

                 vector<ModelFrame*> frames;
                 actor->MyModel->m_pFrameRoot->EnumerateMeshes(frames);

                 for(int i=0;i<frames.size();i++)
                 {
                     if(!frames[i]->GetMesh(0))
                         continue;

                     if(actor->StaticObject)
                         actor->MyWorld->UpdateStaticPRT(frames[i]->GetMesh(0));
                     else
                     {
                         actor->MyWorld->UpdateDynamicPRT(frames[i]->GetMesh(0),frames[i]->CombinedTransformationMatrix,actor->MyModel->m_TouchingLights);
                     }

                     Matrix tm;
                     MeshOps::PRTToStatic(frames[i],editor->m_World);

                     // Regenerate LODs by setting mesh again
                     frames[i]->SetMesh(frames[i]->GetMesh(0));
                 }
             }
         }

private: System::Void viewToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
         {
         }

         /// <summary>
         /// Breaks mesh into sub-meshes based on attribute segments
         /// </summary>
private: System::Void breakMeshintosubelementsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
         {
             vector<Actor*> selected = editor->m_SelectedActors;
             editor->UnSelectAll();

             for(int j=0;j<selected.size();j++)
             {
                 Actor* actor = selected[j];
                 if(!actor || !actor->MyModel)
                     continue;

                 vector<ModelFrame*> frames;
                 actor->MyModel->m_pFrameRoot->EnumerateMeshes(frames);
                 if(frames.size() > 1)
                 {
                     SeriousWarning("Actor %s cannot be broken because it contains multiple sub-frames, which is not supported. Skipping.",actor->m_Name.c_str());
                     continue;
                 }

                 for(int i=0;i<frames.size();i++)
                 {
                     if(!frames[i]->GetMesh(0))
                         continue;

                     vector<Actor*> newActors;
                     MeshOps::Break(frames[i]->GetMesh(0),actor,newActors);
                     for(int k=0;k<newActors.size();k++)
                         editor->SelectActor(newActors[k]);
                 }
             }
         }

};	
}



