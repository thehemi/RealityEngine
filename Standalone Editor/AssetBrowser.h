#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace PropertyBags;


namespace StandaloneEditor
{
    /// <summary> 
    /// Summary for AssetBrowser
    ///
    /// WARNING: If you change the name of this class, you will need to change the 
    ///          'Resource File Name' property for the managed resource compiler tool 
    ///          associated with all .resx files this class depends on.  Otherwise,
    ///          the designers will not be able to interact properly with localized
    ///          resources associated with this form.
    /// </summary>
    public ref class AssetBrowser : public System::Windows::Forms::Form
    {

    public: 
        AssetBrowser(void)
        {
            InitializeComponent();			

            // Vars
            submat = 0;
            me = this;
            editor = Editor::Instance();

            // Classes
            propHelper = gcnew PropertyHelper();
            pgMaterials->SelectedObject = propHelper->class1;

            // Events
            tvMaterials->AfterLabelEdit += gcnew NodeLabelEditEventHandler(this,&AssetBrowser::MaterialRename);
            tabControl1->SelectedIndexChanged += gcnew System::EventHandler(this,&AssetBrowser::TabChanged);
            tvMaterials->AfterSelect += gcnew TreeViewEventHandler(this,&AssetBrowser::MaterialChanged);

            this->Closing += gcnew CancelEventHandler(this, &AssetBrowser::AssetBrowser_Closing);

            // Materials Tree
            TreeNode^ node = gcnew TreeNode("Materials",DIR_IMAGE,DIR_IMAGE);
            node->Tag = gcnew String("..\\Materials\\");
            CreateTree("..\\Materials\\",node,".xml");
            tvMaterials->Nodes->Add(node);
            // Prefabs Tree
            node = gcnew TreeNode("Models",DIR_IMAGE,DIR_IMAGE);
            node->Tag = gcnew String("..\\Models\\");
            CreatePrefabTree("..\\Models\\", node);
            //CreateTree("..\\Models\\",node,".xml");
            tvPrefabs->Nodes->Add(node);
            // Scripts Tree
            node = gcnew TreeNode("Game",DIR_IMAGE,DIR_IMAGE);
            node->Tag = gcnew String("..\\Scripts\\Game\\");
            CreateTree("..\\Scripts\\Game\\",node,".cs");
            tvActors->Nodes->Add(node);
            // Scripts Tree
            node = gcnew TreeNode("Spawnable",DIR_IMAGE,DIR_IMAGE);
            node->Tag = gcnew String("..\\Scripts\\Spawnable\\");
            CreateTree("..\\Scripts\\Spawnable\\",node,".cs");
            tvActors->Nodes->Add(node);				

            //
            thumbArray = gcnew System::Collections::ArrayList();
        }

    protected: 
        void Dispose(Boolean disposing)
        {
            if (disposing && components)
            {
                components->Dispose();
            }
            __super::Dispose(disposing);
        }


    private: System::Windows::Forms::SplitContainer^  splitContainer3;
    private: System::Windows::Forms::ComboBox^  comboTechniques;
    private: System::Windows::Forms::PropertyGrid^  pgMaterials;
    private: System::Windows::Forms::TabPage^  prefabsTab;
    private: System::Windows::Forms::ToolStrip^  toolStrip2;
    private: System::Windows::Forms::ToolStripButton^  toolStripApplySelection;
    private: System::Windows::Forms::ToolStripButton^  toolStripButton3;
    private: System::Windows::Forms::ToolStripButton^  toolSaveMatLib;
    private: System::Windows::Forms::ToolStripButton^  toolStripNewMat;
    private: System::Windows::Forms::ToolStripButton^  deleteSelected;
    private: System::Windows::Forms::ToolStrip^  toolStrip1;
    private: System::Windows::Forms::ToolStrip^  toolStrip3;
    private: System::Windows::Forms::ToolStripButton^  toolStripButton2;
    private: System::Windows::Forms::SplitContainer^  splitContainer4;
    private: System::Windows::Forms::TreeView^  tvPrefabs;
    private: System::Windows::Forms::ToolStripButton^  newPrefabFolder;
    private: System::Windows::Forms::ToolStripButton^  reloadShader;
    private: System::Windows::Forms::TreeView^  tvActors;
    private: System::Windows::Forms::ToolStripButton^  newScript;
    private: System::Windows::Forms::ToolStripButton^  editScript;
    private: System::Windows::Forms::ToolStripButton^  addPrefabToolStripButton;
    private: System::Windows::Forms::SaveFileDialog^  saveFileDialog;
    private: System::Windows::Forms::MenuStrip^  menuStrip1;
    private: System::Windows::Forms::RaftingContainer^  leftRaftingContainer;
    private: System::Windows::Forms::RaftingContainer^  leftRaftingContainer1;
    private: System::Windows::Forms::RaftingContainer^  topRaftingContainer;
    private: System::Windows::Forms::RaftingContainer^  bottomRaftingContainer;
    private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  viewToolStripMenuItem;
    private: System::Windows::Forms::TabControl^  tabControl1;
    private: System::Windows::Forms::TabPage^  actorsTab;
    private: System::Windows::Forms::SplitContainer^  splitContainer0;
    private: System::Windows::Forms::SplitContainer^  splitContainer1;
    private: System::Windows::Forms::TabPage^  materialsTab;
    private: System::Windows::Forms::SplitContainer^  splitContainer2;
    private: System::Windows::Forms::ImageList^  imageList1;
    private: System::Windows::Forms::TreeView^  tvMaterials;
    private: System::Windows::Forms::ToolStrip^  toolsToolbar;
    private: System::Windows::Forms::ImageList^  ToolBarImages;
    private: System::Windows::Forms::ToolStripButton^  moveButton;
    private: System::Windows::Forms::ToolStripButton^  rotateButton;
    private: System::Windows::Forms::ToolStripButton^  scaleButton;
    private: System::Windows::Forms::ToolStripButton^  toolStripButton4;
    private: System::Windows::Forms::ToolStripButton^  orbitViewToolStripButton;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator7;
    private: System::Windows::Forms::ToolStripComboBox^  toolStripComboBox1;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
    private: System::Windows::Forms::ToolStripButton^  slowToolStripButton;
    private: System::Windows::Forms::ToolStripButton^  mediumToolStripButton;
    private: System::Windows::Forms::ToolStripButton^  fastToolStripButton;
private: System::Windows::Forms::ToolStripButton^  replacePrefabToolStripButton;
private: System::Windows::Forms::ToolStripButton^  toolStripButton1;

    private: System::ComponentModel::IContainer^  components;

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
			System::ComponentModel::ComponentResourceManager^  resources = gcnew System::ComponentModel::ComponentResourceManager(typeid<AssetBrowser >);
			this->menuStrip1 = gcnew System::Windows::Forms::MenuStrip();
			this->fileToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->viewToolStripMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->leftRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
			this->leftRaftingContainer1 = gcnew System::Windows::Forms::RaftingContainer();
			this->topRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
			this->bottomRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
			this->tabControl1 = gcnew System::Windows::Forms::TabControl();
			this->materialsTab = gcnew System::Windows::Forms::TabPage();
			this->splitContainer0 = gcnew System::Windows::Forms::SplitContainer();
			this->toolStrip2 = gcnew System::Windows::Forms::ToolStrip();
			this->toolStripApplySelection = gcnew System::Windows::Forms::ToolStripButton();
			this->toolStripButton3 = gcnew System::Windows::Forms::ToolStripButton();
			this->toolSaveMatLib = gcnew System::Windows::Forms::ToolStripButton();
			this->toolStripNewMat = gcnew System::Windows::Forms::ToolStripButton();
			this->toolStripButton2 = gcnew System::Windows::Forms::ToolStripButton();
			this->deleteSelected = gcnew System::Windows::Forms::ToolStripButton();
			this->reloadShader = gcnew System::Windows::Forms::ToolStripButton();
			this->splitContainer1 = gcnew System::Windows::Forms::SplitContainer();
			this->tvMaterials = gcnew System::Windows::Forms::TreeView();
			this->imageList1 = gcnew System::Windows::Forms::ImageList(this->components);
			this->splitContainer2 = gcnew System::Windows::Forms::SplitContainer();
			this->splitContainer3 = gcnew System::Windows::Forms::SplitContainer();
			this->comboTechniques = gcnew System::Windows::Forms::ComboBox();
			this->pgMaterials = gcnew System::Windows::Forms::PropertyGrid();
			this->prefabsTab = gcnew System::Windows::Forms::TabPage();
			this->splitContainer4 = gcnew System::Windows::Forms::SplitContainer();
			this->tvPrefabs = gcnew System::Windows::Forms::TreeView();
			this->toolStrip3 = gcnew System::Windows::Forms::ToolStrip();
			this->newPrefabFolder = gcnew System::Windows::Forms::ToolStripButton();
			this->addPrefabToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
			this->replacePrefabToolStripButton = gcnew System::Windows::Forms::ToolStripButton();
			this->actorsTab = gcnew System::Windows::Forms::TabPage();
			this->tvActors = gcnew System::Windows::Forms::TreeView();
			this->toolStrip1 = gcnew System::Windows::Forms::ToolStrip();
			this->newScript = gcnew System::Windows::Forms::ToolStripButton();
			this->editScript = gcnew System::Windows::Forms::ToolStripButton();
			this->saveFileDialog = gcnew System::Windows::Forms::SaveFileDialog();
			this->toolStripButton1 = gcnew System::Windows::Forms::ToolStripButton();
			this->menuStrip1->SuspendLayout();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer))->BeginInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer1))->BeginInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->topRaftingContainer))->BeginInit();
			this->topRaftingContainer->SuspendLayout();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bottomRaftingContainer))->BeginInit();
			this->tabControl1->SuspendLayout();
			this->materialsTab->SuspendLayout();
			this->splitContainer0->Panel1->SuspendLayout();
			this->splitContainer0->Panel2->SuspendLayout();
			this->splitContainer0->SuspendLayout();
			this->toolStrip2->SuspendLayout();
			this->splitContainer1->Panel1->SuspendLayout();
			this->splitContainer1->Panel2->SuspendLayout();
			this->splitContainer1->SuspendLayout();
			this->splitContainer2->Panel2->SuspendLayout();
			this->splitContainer2->SuspendLayout();
			this->splitContainer3->Panel1->SuspendLayout();
			this->splitContainer3->Panel2->SuspendLayout();
			this->splitContainer3->SuspendLayout();
			this->prefabsTab->SuspendLayout();
			this->splitContainer4->Panel1->SuspendLayout();
			this->splitContainer4->SuspendLayout();
			this->toolStrip3->SuspendLayout();
			this->actorsTab->SuspendLayout();
			this->toolStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(2) {this->fileToolStripMenuItem, this->viewToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Padding = System::Windows::Forms::Padding(6, 2, 0, 2);
			this->menuStrip1->Raft = System::Windows::Forms::RaftingSides::Top;
			this->menuStrip1->TabIndex = 0;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->SettingsKey = L"AssetBrowser.fileToolStripMenuItem";
			this->fileToolStripMenuItem->Text = L"File";
			// 
			// viewToolStripMenuItem
			// 
			this->viewToolStripMenuItem->Name = L"viewToolStripMenuItem";
			this->viewToolStripMenuItem->SettingsKey = L"AssetBrowser.viewToolStripMenuItem";
			this->viewToolStripMenuItem->Text = L"View";
			// 
			// leftRaftingContainer
			// 
			this->leftRaftingContainer->Dock = System::Windows::Forms::DockStyle::Left;
			this->leftRaftingContainer->Name = L"leftRaftingContainer";
			// 
			// leftRaftingContainer1
			// 
			this->leftRaftingContainer1->Dock = System::Windows::Forms::DockStyle::Left;
			this->leftRaftingContainer1->Name = L"leftRaftingContainer1";
			// 
			// topRaftingContainer
			// 
			this->topRaftingContainer->Controls->Add(this->menuStrip1);
			this->topRaftingContainer->Dock = System::Windows::Forms::DockStyle::Top;
			this->topRaftingContainer->Name = L"topRaftingContainer";
			// 
			// bottomRaftingContainer
			// 
			this->bottomRaftingContainer->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->bottomRaftingContainer->Name = L"bottomRaftingContainer";
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->materialsTab);
			this->tabControl1->Controls->Add(this->prefabsTab);
			this->tabControl1->Controls->Add(this->actorsTab);
			this->tabControl1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tabControl1->Location = System::Drawing::Point(0, 21);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->ShowToolTips = true;
			this->tabControl1->Size = System::Drawing::Size(661, 566);
			this->tabControl1->TabIndex = 4;
			// 
			// materialsTab
			// 
			this->materialsTab->Controls->Add(this->splitContainer0);
			this->materialsTab->Location = System::Drawing::Point(4, 22);
			this->materialsTab->Name = L"materialsTab";
			this->materialsTab->Padding = System::Windows::Forms::Padding(3);
			this->materialsTab->Size = System::Drawing::Size(653, 540);
			this->materialsTab->TabIndex = 1;
			this->materialsTab->Text = L"Materials";
			// 
			// splitContainer0
			// 
			this->splitContainer0->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer0->FixedPanel = System::Windows::Forms::FixedPanel::Panel1;
			this->splitContainer0->Location = System::Drawing::Point(3, 3);
			this->splitContainer0->Name = L"splitContainer0";
			this->splitContainer0->Orientation = System::Windows::Forms::Orientation::Horizontal;
			// 
			// Panel1
			// 
			this->splitContainer0->Panel1->Controls->Add(this->toolStrip2);
			// 
			// Panel2
			// 
			this->splitContainer0->Panel2->Controls->Add(this->splitContainer1);
			this->splitContainer0->Size = System::Drawing::Size(647, 534);
			this->splitContainer0->SplitterDistance = 25;
			this->splitContainer0->TabIndex = 0;
			this->splitContainer0->Text = L"splitContainer0";
			// 
			// toolStrip2
			// 
			this->toolStrip2->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(8) {this->toolStripApplySelection, this->toolStripButton3, this->toolSaveMatLib, this->toolStripNewMat, this->toolStripButton2, this->deleteSelected, this->reloadShader, this->toolStripButton1});
			this->toolStrip2->Location = System::Drawing::Point(0, 0);
			this->toolStrip2->Name = L"toolStrip2";
			this->toolStrip2->Raft = System::Windows::Forms::RaftingSides::None;
			this->toolStrip2->TabIndex = 12;
			this->toolStrip2->Text = L"toolStrip2";
			// 
			// toolStripApplySelection
			// 
			this->toolStripApplySelection->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripApplySelection.Image")));
			this->toolStripApplySelection->Name = L"toolStripApplySelection";
			this->toolStripApplySelection->SettingsKey = L"MainForm.toolStripButton2";
			this->toolStripApplySelection->ToolTipText = L"Apply To Selection";
			this->toolStripApplySelection->Click += gcnew System::EventHandler(this, &AssetBrowser::toolStripApplySelection_Click);
			// 
			// toolStripButton3
			// 
			this->toolStripButton3->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton3.Image")));
			this->toolStripButton3->Name = L"toolStripButton3";
			this->toolStripButton3->SettingsKey = L"MainForm.toolStripButton3";
			this->toolStripButton3->ToolTipText = L"Get Material From Selected";
			this->toolStripButton3->Click += gcnew System::EventHandler(this, &AssetBrowser::toolStripButton3_Click);
			// 
			// toolSaveMatLib
			// 
			this->toolSaveMatLib->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolSaveMatLib.Image")));
			this->toolSaveMatLib->Name = L"toolSaveMatLib";
			this->toolSaveMatLib->SettingsKey = L"MainForm.toolStripButton4";
			this->toolSaveMatLib->ToolTipText = L"Save All Modified Materials";
			this->toolSaveMatLib->Click += gcnew System::EventHandler(this, &AssetBrowser::toolStripButton4_Click);
			// 
			// toolStripNewMat
			// 
			this->toolStripNewMat->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripNewMat.Image")));
			this->toolStripNewMat->Name = L"toolStripNewMat";
			this->toolStripNewMat->SettingsKey = L"MainForm.toolStripButton5";
			this->toolStripNewMat->ToolTipText = L"New Material";
			this->toolStripNewMat->Click += gcnew System::EventHandler(this, &AssetBrowser::toolStripButton5_Click);
			// 
			// toolStripButton2
			// 
			this->toolStripButton2->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton2.Image")));
			this->toolStripButton2->Name = L"toolStripButton2";
			this->toolStripButton2->SettingsKey = L"MainForm.toolStripButton6";
			this->toolStripButton2->ToolTipText = L"New Folder";
			this->toolStripButton2->Click += gcnew System::EventHandler(this, &AssetBrowser::toolStripButton6_Click);
			// 
			// deleteSelected
			// 
			this->deleteSelected->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"deleteSelected.Image")));
			this->deleteSelected->Name = L"deleteSelected";
			this->deleteSelected->SettingsKey = L"AssetBrowser.toolStripButton1";
			this->deleteSelected->ToolTipText = L"Delete Selected";
			this->deleteSelected->Click += gcnew System::EventHandler(this, &AssetBrowser::deleteSelected_Click);
			// 
			// reloadShader
			// 
			this->reloadShader->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"reloadShader.Image")));
			this->reloadShader->Name = L"reloadShader";
			this->reloadShader->SettingsKey = L"AssetBrowser.toolStripButton4";
			this->reloadShader->ToolTipText = L"Reload Shader";
			this->reloadShader->Click += gcnew System::EventHandler(this, &AssetBrowser::reloadShader_Click);
			// 
			// splitContainer1
			// 
			this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer1->FixedPanel = System::Windows::Forms::FixedPanel::Panel1;
			this->splitContainer1->Location = System::Drawing::Point(0, 0);
			this->splitContainer1->Name = L"splitContainer1";
			// 
			// Panel1
			// 
			this->splitContainer1->Panel1->Controls->Add(this->tvMaterials);
			// 
			// Panel2
			// 
			this->splitContainer1->Panel2->Controls->Add(this->splitContainer2);
			this->splitContainer1->Size = System::Drawing::Size(647, 505);
			this->splitContainer1->SplitterDistance = 122;
			this->splitContainer1->TabIndex = 0;
			this->splitContainer1->Text = L"splitContainer1";
			// 
			// tvMaterials
			// 
			this->tvMaterials->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tvMaterials->HideSelection = false;
			this->tvMaterials->ImageIndex = 1;
			this->tvMaterials->ImageList = this->imageList1;
			this->tvMaterials->LabelEdit = true;
			this->tvMaterials->Location = System::Drawing::Point(0, 0);
			this->tvMaterials->Name = L"tvMaterials";
			this->tvMaterials->Size = System::Drawing::Size(122, 505);
			this->tvMaterials->Sorted = true;
			this->tvMaterials->TabIndex = 0;
			this->tvMaterials->AfterSelect += gcnew System::Windows::Forms::TreeViewEventHandler(this, &AssetBrowser::tvMaterials_AfterSelect);
			// 
			// imageList1
			// 
			this->imageList1->ImageSize = System::Drawing::Size(16, 16);
			this->imageList1->ImageStream = (stdcli::language::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"imageList1.ImageStream")));
			this->imageList1->TransparentColor = System::Drawing::Color::Transparent;
			this->imageList1->Images->SetKeyName(0, L"folder_open.gif");
			this->imageList1->Images->SetKeyName(1, L"folder_closed.gif");
			this->imageList1->Images->SetKeyName(2, L"mat.gif");
			// 
			// splitContainer2
			// 
			this->splitContainer2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->splitContainer2->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer2->Location = System::Drawing::Point(0, 0);
			this->splitContainer2->Name = L"splitContainer2";
			this->splitContainer2->Orientation = System::Windows::Forms::Orientation::Horizontal;
			// 
			// Panel1
			// 
			this->splitContainer2->Panel1->BackColor = System::Drawing::SystemColors::Control;
			this->splitContainer2->Panel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &AssetBrowser::splitContainer2_Panel1_Paint);
			// 
			// Panel2
			// 
			this->splitContainer2->Panel2->Controls->Add(this->splitContainer3);
			this->splitContainer2->Size = System::Drawing::Size(521, 505);
			this->splitContainer2->SplitterDistance = 25;
			this->splitContainer2->TabIndex = 0;
			this->splitContainer2->Text = L"splitContainer2";
			// 
			// splitContainer3
			// 
			this->splitContainer3->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer3->FixedPanel = System::Windows::Forms::FixedPanel::Panel1;
			this->splitContainer3->Location = System::Drawing::Point(0, 0);
			this->splitContainer3->Name = L"splitContainer3";
			this->splitContainer3->Orientation = System::Windows::Forms::Orientation::Horizontal;
			// 
			// Panel1
			// 
			this->splitContainer3->Panel1->Controls->Add(this->comboTechniques);
			this->splitContainer3->Panel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &AssetBrowser::splitContainer3_Panel1_Paint);
			// 
			// Panel2
			// 
			this->splitContainer3->Panel2->Controls->Add(this->pgMaterials);
			this->splitContainer3->Size = System::Drawing::Size(517, 472);
			this->splitContainer3->SplitterDistance = 25;
			this->splitContainer3->SplitterWidth = 2;
			this->splitContainer3->TabIndex = 0;
			this->splitContainer3->Text = L"splitContainer3";
			// 
			// comboTechniques
			// 
			this->comboTechniques->Dock = System::Windows::Forms::DockStyle::Fill;
			this->comboTechniques->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboTechniques->FormattingEnabled = true;
			this->comboTechniques->Location = System::Drawing::Point(0, 0);
			this->comboTechniques->Name = L"comboTechniques";
			this->comboTechniques->Size = System::Drawing::Size(517, 21);
			this->comboTechniques->TabIndex = 3;
			this->comboTechniques->SelectedIndexChanged += gcnew System::EventHandler(this, &AssetBrowser::comboBox1_SelectedIndexChanged);
			// 
			// pgMaterials
			// 
			this->pgMaterials->CommandsVisibleIfAvailable = true;
			this->pgMaterials->Dock = System::Windows::Forms::DockStyle::Fill;
			this->pgMaterials->Location = System::Drawing::Point(0, 0);
			this->pgMaterials->Name = L"pgMaterials";
			this->pgMaterials->Size = System::Drawing::Size(517, 445);
			this->pgMaterials->TabIndex = 4;
			// 
			// prefabsTab
			// 
			this->prefabsTab->Controls->Add(this->splitContainer4);
			this->prefabsTab->Controls->Add(this->toolStrip3);
			this->prefabsTab->Location = System::Drawing::Point(4, 22);
			this->prefabsTab->Name = L"prefabsTab";
			this->prefabsTab->Padding = System::Windows::Forms::Padding(3);
			this->prefabsTab->Size = System::Drawing::Size(653, 540);
			this->prefabsTab->TabIndex = 2;
			this->prefabsTab->Text = L"Prefabs";
			// 
			// splitContainer4
			// 
			this->splitContainer4->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer4->Location = System::Drawing::Point(3, 28);
			this->splitContainer4->Name = L"splitContainer4";
			// 
			// Panel1
			// 
			this->splitContainer4->Panel1->Controls->Add(this->tvPrefabs);
			// 
			// Panel2
			// 
			this->splitContainer4->Panel2->AutoScroll = true;
			this->splitContainer4->Size = System::Drawing::Size(647, 509);
			this->splitContainer4->SplitterDistance = 190;
			this->splitContainer4->TabIndex = 1;
			this->splitContainer4->Text = L"splitContainer4";
			// 
			// tvPrefabs
			// 
			this->tvPrefabs->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tvPrefabs->ImageList = this->imageList1;
			this->tvPrefabs->LabelEdit = true;
			this->tvPrefabs->Location = System::Drawing::Point(0, 0);
			this->tvPrefabs->Name = L"tvPrefabs";
			this->tvPrefabs->Size = System::Drawing::Size(190, 509);
			this->tvPrefabs->TabIndex = 0;
			this->tvPrefabs->AfterSelect += gcnew System::Windows::Forms::TreeViewEventHandler(this, &AssetBrowser::tvPrefabs_AfterSelect);
			// 
			// toolStrip3
			// 
			this->toolStrip3->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(3) {this->newPrefabFolder, this->addPrefabToolStripButton, this->replacePrefabToolStripButton});
			this->toolStrip3->Location = System::Drawing::Point(3, 3);
			this->toolStrip3->Name = L"toolStrip3";
			this->toolStrip3->Raft = System::Windows::Forms::RaftingSides::None;
			this->toolStrip3->TabIndex = 0;
			this->toolStrip3->Text = L"toolStrip3";
			this->toolStrip3->Click += gcnew System::EventHandler(this, &AssetBrowser::toolStrip3_Click);
			// 
			// newPrefabFolder
			// 
			this->newPrefabFolder->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"newPrefabFolder.Image")));
			this->newPrefabFolder->Name = L"newPrefabFolder";
			this->newPrefabFolder->SettingsKey = L"AssetBrowser.toolStripButton4";
			this->newPrefabFolder->Click += gcnew System::EventHandler(this, &AssetBrowser::newPrefabFolder_Click);
			// 
			// addPrefabToolStripButton
			// 
			this->addPrefabToolStripButton->Name = L"addPrefabToolStripButton";
			this->addPrefabToolStripButton->SettingsKey = L"AssetBrowser.addPrefabToolStripButton";
			this->addPrefabToolStripButton->Text = L"Add Prefab";
			this->addPrefabToolStripButton->Click += gcnew System::EventHandler(this, &AssetBrowser::addPrefabToolStripButton_Click);
			// 
			// replacePrefabToolStripButton
			// 
			this->replacePrefabToolStripButton->Name = L"replacePrefabToolStripButton";
			this->replacePrefabToolStripButton->SettingsKey = L"AssetBrowser.replacePrefabToolStripButton";
			this->replacePrefabToolStripButton->Text = L"Replace Prefab";
			this->replacePrefabToolStripButton->Click += gcnew System::EventHandler(this, &AssetBrowser::replacePrefabToolStripButton_Click);
			// 
			// actorsTab
			// 
			this->actorsTab->Controls->Add(this->tvActors);
			this->actorsTab->Controls->Add(this->toolStrip1);
			this->actorsTab->Location = System::Drawing::Point(4, 22);
			this->actorsTab->Name = L"actorsTab";
			this->actorsTab->Padding = System::Windows::Forms::Padding(3);
			this->actorsTab->Size = System::Drawing::Size(653, 540);
			this->actorsTab->TabIndex = 0;
			this->actorsTab->Text = L"Actors";
			// 
			// tvActors
			// 
			this->tvActors->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tvActors->Location = System::Drawing::Point(3, 28);
			this->tvActors->Name = L"tvActors";
			this->tvActors->Size = System::Drawing::Size(647, 509);
			this->tvActors->TabIndex = 1;
			// 
			// toolStrip1
			// 
			this->toolStrip1->Items->AddRange(gcnew stdcli::language::array<System::Windows::Forms::ToolStripItem^ >(2) {this->newScript, this->editScript});
			this->toolStrip1->Location = System::Drawing::Point(3, 3);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->Raft = System::Windows::Forms::RaftingSides::None;
			this->toolStrip1->TabIndex = 0;
			this->toolStrip1->Text = L"toolStrip1";
			// 
			// newScript
			// 
			this->newScript->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"newScript.Image")));
			this->newScript->Name = L"newScript";
			this->newScript->SettingsKey = L"AssetBrowser.toolStripButton1";
			this->newScript->ToolTipText = L"New Script";
			// 
			// editScript
			// 
			this->editScript->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"editScript.Image")));
			this->editScript->Name = L"editScript";
			this->editScript->SettingsKey = L"AssetBrowser.toolStripButton4";
			this->editScript->ToolTipText = L"Edit Script";
			this->editScript->Click += gcnew System::EventHandler(this, &AssetBrowser::editScript_Click);
			// 
			// saveFileDialog
			// 
			this->saveFileDialog->DefaultExt = L"xml";
			this->saveFileDialog->Filter = L"XML Files|*.xml|All files|*.*";
			this->saveFileDialog->FilterIndex = 2;
			this->saveFileDialog->Title = L"Select Asset Save Location...";
			// 
			// toolStripButton1
			// 
			this->toolStripButton1->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"toolStripButton1.Image")));
			this->toolStripButton1->Name = L"toolStripButton1";
			this->toolStripButton1->SettingsKey = L"AssetBrowser.toolStripButton1";
			this->toolStripButton1->Click += gcnew System::EventHandler(this, &AssetBrowser::toolStripButton1_Click);
			// 
			// AssetBrowser
			// 
			this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
			this->ClientSize = System::Drawing::Size(661, 587);
			this->Controls->Add(this->tabControl1);
			this->Controls->Add(this->leftRaftingContainer);
			this->Controls->Add(this->leftRaftingContainer1);
			this->Controls->Add(this->topRaftingContainer);
			this->Controls->Add(this->bottomRaftingContainer);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"AssetBrowser";
			this->ShowInTaskbar = false;
			this->Text = L"AssetBrowser";
			this->menuStrip1->ResumeLayout(false);
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer))->EndInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer1))->EndInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->topRaftingContainer))->EndInit();
			this->topRaftingContainer->ResumeLayout(false);
			this->topRaftingContainer->PerformLayout();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bottomRaftingContainer))->EndInit();
			this->tabControl1->ResumeLayout(false);
			this->materialsTab->ResumeLayout(false);
			this->splitContainer0->Panel1->ResumeLayout(false);
			this->splitContainer0->Panel1->PerformLayout();
			this->splitContainer0->Panel2->ResumeLayout(false);
			this->splitContainer0->ResumeLayout(false);
			this->toolStrip2->ResumeLayout(false);
			this->splitContainer1->Panel1->ResumeLayout(false);
			this->splitContainer1->Panel2->ResumeLayout(false);
			this->splitContainer1->ResumeLayout(false);
			this->splitContainer2->Panel2->ResumeLayout(false);
			this->splitContainer2->ResumeLayout(false);
			this->splitContainer3->Panel1->ResumeLayout(false);
			this->splitContainer3->Panel2->ResumeLayout(false);
			this->splitContainer3->ResumeLayout(false);
			this->prefabsTab->ResumeLayout(false);
			this->prefabsTab->PerformLayout();
			this->splitContainer4->Panel1->ResumeLayout(false);
			this->splitContainer4->ResumeLayout(false);
			this->toolStrip3->ResumeLayout(false);
			this->actorsTab->ResumeLayout(false);
			this->actorsTab->PerformLayout();
			this->toolStrip1->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}		
#pragma endregion


    public:

        static AssetBrowser^ me;
        PropertyHelper^ propHelper;
        static int DIR_IMAGE = 1;
        static int MAT_IMAGE = 2;
        Editor*	editor;
        System::Collections::ArrayList^ thumbArray;
    private: 
        TreeNode^  lastNode;

    public: void ResetPrefabTree()
            {
                tvPrefabs->Nodes->Clear();
                TreeNode^ node = gcnew TreeNode("Models",DIR_IMAGE,DIR_IMAGE);
                node->Tag = gcnew String("..\\Models\\");
                CreatePrefabTree("..\\Models\\", node);						
                tvPrefabs->Nodes->Add(node);	
            };

    private: System::Void AssetBrowser_Closing(System::Object^  sender, CancelEventArgs^ e)
             {
                 e->Cancel = true;
                 this->Hide();
             }

             int submat;
    public: void SetSubMatIndex(int index)
            {
                if(index == -1)
                    index = 0;
                submat = index;
            }

             int submesh;
    public: void SetSubMeshIndex(int index)
            {
                if(index == -1)
                    index = 0;
                submesh = index;
            }
            /// <summary>
            /// Updates material based on selection
            /// </summary>
    public: MatWrapper^ selectedMat;

    public:  void tvPrefabs_AfterSelect(Object^ sender, TreeViewEventArgs^ args)
             {
                 if(!tvPrefabs->SelectedNode)
                     return;

                 ClearThumbs();

                 if(tvPrefabs->SelectedNode->ImageIndex != DIR_IMAGE)
                     return;

                 String^ str = (String^)tvPrefabs->SelectedNode->Tag;

                 this->prefabsTab->SuspendLayout();
                 this->splitContainer4->Panel1->SuspendLayout();
                 this->splitContainer4->SuspendLayout();


                 LoadThumbs(ToCppString(str),0, 0, 140,  140, 3,  
                     splitContainer4->Panel2);

                 this->prefabsTab->ResumeLayout(false);
                 this->prefabsTab->PerformLayout();
                 this->splitContainer4->Panel1->ResumeLayout(false);
                 this->splitContainer4->ResumeLayout(false);	



                 // silly hack... 
                 // this->AdjustFormScrollbars(true); doesn't seem to work
                 // nor this->Invalidate();

                 static bool WichWaySwitch=false;				 				
                 System::Drawing::Size size=this->Size;
                 if(WichWaySwitch)
                 {
                     size.Height--;
                     size.Width--;
                 }
                 else
                 {
                     size.Height++;
                     size.Width++;
                 };

                 WichWaySwitch=!WichWaySwitch;					
                 this->Size=size; 				
             };


    public:  void MaterialChanged(Object^ sender, TreeViewEventArgs^ args)
             {
                 if(!tvMaterials->SelectedNode)
                     return;

                 // Return if this tab isn't selected
                 if(tabControl1->SelectedIndex != 0)
                     return;

                 propHelper->class1->Properties->Clear();
                 comboTechniques->Items->Clear();

                 // Invalid node, such as folder, return
                 if(tvMaterials->SelectedNode->ImageIndex != MAT_IMAGE || !tvMaterials->SelectedNode->Tag)
                 {
                     return;
                 }

                 selectedMat = stdcli::language::safe_cast<MatWrapper^>(tvMaterials->SelectedNode->Tag);

                 // Does material need loading for first time?
                 if(selectedMat->file && !selectedMat->mat)
                 {
                     // Load material!
                     selectedMat->mat = new Material;

                     String^ str = (String^)tvMaterials->SelectedNode->Parent->Tag;
                     if(!selectedMat->mat->Load(ToCppString(String::Concat(str,selectedMat->file) ).c_str() ) )
                     {
                         String^ err = gcnew String("Failed to load material: ");
                         err = String::Concat(err,selectedMat->file);

                         MessageBox::Show(this, err, "Material Loading Failed", MessageBoxButtons::OK,
                             MessageBoxIcon::Warning, MessageBoxDefaultButton::Button1, MessageBoxOptions::RightAlign);

                         SAFE_RELEASE(selectedMat->mat);

                     }

                 }

                 // Material loaded, fill parameters
                 if(selectedMat->mat)
                 {
                     Material* mat = selectedMat->mat;
                     //
                     // Fill techniques
                     //
                     int techIndex = 0;
                     for(int i=0;i<mat->m_TechniqueNames.size();i++){
                         comboTechniques->Items->Add(gcnew String(mat->m_TechniqueNames[i].c_str()));

                         // Set current index to shader's current technique
                         if(mat->m_TechniqueNames[i].find(mat->m_Token)){
                             techIndex = i;
                         }
                     }
                     comboTechniques->SelectedIndex = techIndex;

                     //
                     // Fill rendering properties
                     //
                     propHelper->UpdateVars(mat->EditorVars);
                     pgMaterials->SelectedObject = propHelper->class1;
                 }
             }

             /// Tab changed
             void TabChanged(Object^ sender, EventArgs^ args)
             {
                 // Changed to materials
                 if(tabControl1->SelectedIndex == 0)
                 {

                 }

                 // Changed to actors
                 if(tabControl1->SelectedIndex == 1)
                 {


                 }
             }

             /// Technique changed
    private: System::Void comboBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 //if(updatingUI)
                 //	 return;

                 Material* m = selectedMat->mat;
                 if(m)
                     m->SetTechnique(ToCppString(comboTechniques->SelectedItem->ToString()));
             }

             /// <summary>
             /// New material folder
             /// </summary>
    private: System::Void toolStripButton6_Click(System::Object^  sender, System::EventArgs^  e)
             {
#undef CreateDirectory
                 String ^dir = "..\\Materials\\New Folder";
                 System::IO::Directory::CreateDirectory(dir);
                 TreeNode^ node = gcnew TreeNode("New Folder",DIR_IMAGE,DIR_IMAGE);
                 // Store full path in tag
                 node->Tag = dir;
                 tvMaterials->Nodes->Add(node);
             }



             /// <summary>
             /// Apply material to selected
             /// </summary>
    private: System::Void toolStripApplySelection_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(!selectedMat || !selectedMat->mat)
                     return;

                 vector<ModelFrame*> selectedMeshes;
                 Editor* editor = Editor::Instance();
                 for(int i=0;i<editor->m_SelectedActors.size();i++)
                 {
                     if(editor->m_SelectedActors[i]->MyModel && editor->m_SelectedActors[i]->MyModel->m_pFrameRoot)
                         editor->m_SelectedActors[i]->MyModel->m_pFrameRoot->EnumerateMeshes(selectedMeshes);
                 }

                 if(selectedMeshes.size() <= submesh)
                     return;

                 vector<Material*> matsToUndo;
                 int	subMatToUndo=submat;

                 int offset = 0;
                 //for(int i=0;i<selectedMeshes.size();i++)
                 {
                     Mesh* m =  selectedMeshes[submesh]->GetMesh();

                     // Apply to submat
                     if(submat < m->m_Materials.size())
                     {
                         // Dave:
                         // the add references remain until undo hits them
                         // even material used will have a ref, so even if it is
                         // changed to another, that ref will be associated with the undo
                         // and be released once the undo it self is released

                         // seems counter intuitives to have references be for undos
                         // but this avoids material being changes and materials being 
                         // reloaded for undo's
                         selectedMat->mat->AddRef();

                         // Dave: don't release here, leave it up for old undo hitting max
                         // m->m_Materials[submat]->Release();
                         matsToUndo.push_back(m->m_Materials[submat]);

                         m->m_Materials[submat] = selectedMat->mat;
                     }
                    // offset += m->m_Materials.size();
                 }

      //           editor->PushUndo( Editor::Undo(editor->m_SelectedActors,matsToUndo,
      //               subMatToUndo, Editor::Undo::ApplySelectionAsset));	
             }

             // Get material(s) FROM selected
    private: System::Void toolStripButton3_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 vector<ModelFrame*> selectedMeshes;
                 Editor* editor = Editor::Instance();
                 for(int i=0;i<editor->m_SelectedActors.size();i++)
                 {
                     if(editor->m_SelectedActors[i]->MyModel && editor->m_SelectedActors[i]->MyModel->m_pFrameRoot)
                         editor->m_SelectedActors[i]->MyModel->m_pFrameRoot->EnumerateMeshes(selectedMeshes);
                 }

                 if(selectedMeshes.size() <= submesh || !selectedMeshes[submesh]->GetMesh()->m_Materials.size())
                     return;

                 //for(int i=0;i<selectedMeshes.size();i++)
                 {
                     Material* mat = 0;

                     // FIXME: Won't work
                     if(submat<selectedMeshes[submesh]->GetMesh()->m_Materials.size())
                         mat = selectedMeshes[submesh]->GetMesh()->m_Materials[submat];

                     TreeNode^ node = gcnew TreeNode(gcnew String(mat->m_Name.c_str()),MAT_IMAGE,MAT_IMAGE);
                     // Parent from another node IF a folder
                     if(tvMaterials->SelectedNode && tvMaterials->SelectedNode->ImageIndex != MAT_IMAGE)
                         tvMaterials->SelectedNode->Nodes->Add(node);
                     // Make root node
                     else
                         tvMaterials->Nodes->Add(node);

                     // Wrap material
                     MatWrapper^ wrapper = gcnew MatWrapper();
                     wrapper->mat = mat;
                     node->Tag = wrapper;
                 }
             }

             /// <summary>
             /// Rename material when GUI rename occurs
             /// </summary>
             void MaterialRename(Object^ sender, NodeLabelEditEventArgs^ args)
             {
                 if(!args || !args->Node || !args->Node->Tag || !args->Label)
                     return;

                 // Folder
                 if(tvMaterials->SelectedNode->ImageIndex == DIR_IMAGE)
                 {
                     String^ dir = (String^)args->Node->Tag;
                     dir = dir->TrimEnd('\\');
                     String^ folder = dir->Substring(dir->LastIndexOf("\\")+1);
                     String^ path   = dir->Substring(0,dir->LastIndexOf("\\")+1);
                     args->Node->Tag = String::Concat(path,args->Label);

                     System::IO::Directory::Move(dir, (String^)args->Node->Tag);
                 }
                 // Material
                 else{
                     Material* m = ((MatWrapper^)args->Node->Tag)->mat;
                     if(m)
                         m->m_Name = ToCppString(args->Label);
                 }
             }

             /// <summary>
             /// Node recursive save materials to file
             /// </summary>
    private: System::Void SaveMaterials(TreeNode^ node)
             {
                 for(int i=0;i<node->Nodes->Count;i++)
                 {
                     SaveMaterials(node->Nodes[i]);
                 }

                 if(!node->Tag || node->ImageIndex != MAT_IMAGE)
                     return;

                 MatWrapper^ mat = (MatWrapper^)node->Tag;
                 if(!mat->mat)
                     return;

                 // If there's an old file, and if it's been renamed, delete it
                 if(mat->file)
                 {
                     if(ToCppString(mat->file) != (mat->mat->m_Name + ".xml"))
                     {
                         // Renamed, delete file!
                         String^ file = (String^)node->Parent->Tag;
                         file = String::Concat(file,mat->file);
                         System::IO::File::Delete(file);
                     }
                 }

                 string file;
                 if(node->Parent && node->Parent->Tag)
                     file += ToCppString((String^)node->Parent->Tag); 
                 else
                     file += "..\\Materials\\";
                 file += mat->mat->m_Name+".xml";
                 mat->mat->Save(file);
             }

             /// <summary>
             /// Save materials to file
             /// </summary>
    private: System::Void toolStripButton4_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 ResetCurrentDirectory();

                 // Save all materials
                 for(int i=0;i<tvMaterials->Nodes->Count;i++)
                     SaveMaterials(tvMaterials->Nodes[i]);			
             }

             /// <summary>
             /// New material
             /// </summary>
    private: System::Void toolStripButton5_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 TreeNode^ node = gcnew TreeNode("New Material",MAT_IMAGE,MAT_IMAGE);
                 // Parent from another node IF a folder
                 if(tvMaterials->SelectedNode && tvMaterials->SelectedNode->ImageIndex != MAT_IMAGE)
                     tvMaterials->SelectedNode->Nodes->Add(node);
                 // Make root node
                 else
                     tvMaterials->Nodes->Add(node);

                 // Create new material associated with node
                 Material* m = new Material;
                 m->m_Name = "New Material";
                 m->m_Category = ToCppString(node->Text);
                 CoCreateGuid(&m->m_GUID);

                 // Create diffuse 
                 Texture* t = new Texture;
                 t->Load("DefaultTexture.dds");
                 if(!t->IsValid())
                     Error("Couldn't find/load DefaultTexture.dds\nYour install may be corrupt");
                 // Add to params as 'tDiffuse0'
                 m->m_Parameters.push_back(
                     new ShaderVar("tDiffuse0",EditorVar::TEXTURE,ShaderVar::VAR_DELETE,t));

                 // Create bump
                 t = new Texture;
                 t->Load("DefaultNormal.dds",TT_NORMALMAP);
                 if(!t->IsValid())
                     Error("Couldn't find/load DefaultNormal.dds\nYour install may be corrupt");
                 // Add to params as 'tBump0'
                 m->m_Parameters.push_back(
                     new ShaderVar("tBump0",EditorVar::TEXTURE,ShaderVar::VAR_DELETE,t));

                 // TEMP: Until we hook up loading
                 m->Initialize("Diffuse.fx","Diffuse_Point");
                 m->ExtractParameters();

                 MatWrapper^ wrapper = gcnew MatWrapper;
                 wrapper->mat = m;
                 node->Tag = wrapper;
             }

    private: System::Void toolNewMatLib_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void tvMaterials_AfterSelect(System::Object^  sender, System::Windows::Forms::TreeViewEventArgs^  e)
             {
             }
    private: System::Void splitContainer3_Panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }

             /// <summary>
             /// Deletes selected material or folder from disk and GUI
             /// </summary>
    private: System::Void deleteSelected_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(!tvMaterials->SelectedNode)
                     return;

                 ResetCurrentDirectory();

                 try
                 {
                     if(tvMaterials->SelectedNode->ImageIndex == DIR_IMAGE)
                     {
                         System::IO::Directory::Delete((String^)tvMaterials->SelectedNode->Tag,true);
                     }
                     else
                     {
                         if(tvMaterials->SelectedNode && tvMaterials->SelectedNode->Parent && tvMaterials->SelectedNode->Parent->Tag)
                         {
                             MatWrapper^ mat = (MatWrapper^)tvMaterials->SelectedNode->Tag;
                             String^ file = (String^)tvMaterials->SelectedNode->Parent->Tag;
                             file = String::Concat(file,gcnew String((mat->mat->m_Name+".xml").c_str()));
                             System::IO::File::Delete(file);
                         }
                     }
                 }
                 catch(Exception^ ex){}

                 tvMaterials->SelectedNode->Remove();
             }

    private: System::Void splitContainer2_Panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
             {
             }

    private: System::Void reloadShader_Click(System::Object^  sender, System::EventArgs^  e)
             {
				 if(!selectedMat || !selectedMat->mat)
					 return;

                 selectedMat->mat->Initialize(selectedMat->mat->m_ShaderName.c_str(),"",true);
                 MaterialChanged(nullptr,nullptr);
             }

             /// <summary>
             /// Launches the editor for a script
             /// </summary>
    private: System::Void editScript_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(!tvActors->SelectedNode)
                     return;

                 if(tvActors->SelectedNode->ImageIndex != DIR_IMAGE)
                 {
                     if(tvActors->SelectedNode->Parent && tvActors->SelectedNode->Parent->Tag)
                     {
                         MatWrapper^ node = (MatWrapper^)tvActors->SelectedNode->Tag;
                         String^ file = (String^)tvActors->SelectedNode->Parent->Tag;
                         file = String::Concat(file,node->file);
                         System::Diagnostics::Process::Start("Wordpad.exe",file);
                     }
                 }
             }

             // When a thumb is changed its even handler calls this form function 
             // Clear all to black the eventhandler for click done always sets to blue
    public: System::Void ThumbChanged()
            {				
                this->prefabsTab->SuspendLayout();
                this->splitContainer4->Panel1->SuspendLayout();
                this->splitContainer4->SuspendLayout();

                for(int i=0;i<thumbArray->Count;i++)				
                    ((ThumbNailButton^)thumbArray->Item[i])->ClearToBlack();

                this->prefabsTab->ResumeLayout(false);
                this->prefabsTab->PerformLayout();
                this->splitContainer4->Panel1->ResumeLayout(false);
                this->splitContainer4->ResumeLayout(false);
            };

    private: System::Void newPrefabFolder_Click(System::Object^  sender, System::EventArgs^  e)
             {
             }

    public: System::Void addPrefabToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 if(!tvPrefabs->SelectedNode)
                     return;				 

                 if(tvPrefabs->SelectedNode->ImageIndex == DIR_IMAGE)
                 {
                     for(int i=0;i<thumbArray->Count;i++)	
                     {
                         if(	 ((ThumbNailButton^)thumbArray->Item[i])->IsSet() )
                         {
                             editor->MergeSceneUsingCam(ToCppString(((ThumbNailButton^)thumbArray->Item[i])->fileName));
                             break;
                         }
                     };
                 }
                 //assume individual one is selected
                 else
                 {
                     editor->MergeSceneUsingCam(ToCppString((String ^)tvPrefabs->SelectedNode->Tag));
                 };

                 editor->m_World->RegenerateOcclusionTree();
             }

             /// <summary>
             /// Fills a Tree with a directory structure (Recursive)
             /// </summary>
    private: System::Void CreateTree(string path, TreeNode^ parent, const char* ext)
             {
                 ResetCurrentDirectory();

                 // Add all dir files
                 vector<string> files;
                 enumerateFiles(path.c_str(),files,0,ext);
                 for(int j=0;j<files.size();j++)
                 {
                     string file = files[j].substr(0,files[j].find_last_of("."));
                     TreeNode^ matNode = gcnew TreeNode(gcnew String(file.c_str()),MAT_IMAGE,MAT_IMAGE);

                     // Create class to store meta-info
                     MatWrapper^ wrapper = gcnew MatWrapper;
                     wrapper->file = gcnew String(files[j].c_str());
                     matNode->Tag = wrapper;//reinterpret_cast<Object^>(wrapper);

                     parent->Nodes->Add(matNode);
                 }

                 vector<string> dirs;
                 enumerateDirectories(path.c_str(),dirs,2);
                 for(int i=0;i<dirs.size();i++)
                 {
                     string dir = dirs[i].substr(0,dirs[i].length()-1);
                     dir = dir.substr(dir.find_last_of("\\")+1);

                     // Create Node
                     TreeNode^ node = gcnew TreeNode(gcnew String(dir.c_str()),DIR_IMAGE,DIR_IMAGE);
                     node->Tag = gcnew String(dirs[i].c_str()); // Store full path in tag

                     // Add any subdirectories
                     CreateTree(dirs[i],node,ext);

                     // Add to parent
                     parent->Nodes->Add(node);
                 }	
             }

    private: bool DirectoryHasXML(string path)
             {
                 ResetCurrentDirectory();				
                 vector<string> files;
                 enumerateFiles(path.c_str(),files,0,".xml");				 
                 return (!files.empty());
             };

    private: String^ GetNameXML(string path)
             {
                 ResetCurrentDirectory();				
                 vector<string> files;
                 enumerateFiles(path.c_str(),files,0,".xml");				 
                 return gcnew String(files[0].substr(0,files[0].find_last_of(".")).c_str());
             };

    private: System::Void CreatePrefabTree(string path, TreeNode^ parent)
             {
                 ResetCurrentDirectory();

                 vector<string> dirs;
                 enumerateDirectories(path.c_str(),dirs,2);
                 for(int i=0;i<dirs.size();i++)
                 {
                     string dir = dirs[i].substr(0,dirs[i].length()-1);
                     dir = dir.substr(dir.find_last_of("\\")+1);

                     // Create Node
                     TreeNode^ node = gcnew TreeNode(gcnew String(dir.c_str()),DIR_IMAGE,DIR_IMAGE);
                     node->Tag = gcnew String(dirs[i].c_str()); // Store full path in tag

                     //we know next directory has xml
                     if(DirectoryHasXML(dirs[i]))
                     {
                         string XMLName = ToCppString(GetNameXML(dirs[i]));

                         if(!XMLName.empty())
                         {
                             TreeNode^ prefabNode = gcnew TreeNode(gcnew String(XMLName.c_str()),
                                 MAT_IMAGE,MAT_IMAGE);

                             // Create class to store meta-info							
                             string fullInfo = dirs[i];
                             fullInfo.append(XMLName);
                             fullInfo.append(".xml");

                             prefabNode->Tag = gcnew String(fullInfo.c_str());

                             //parent->Nodes->Add(prefabNode);
                         };
                     }
                     else
                     {
                         // Add any subdirectories
                         parent->Nodes->Add(node);
                         CreatePrefabTree(dirs[i],node);
                     };					 
                 }	
             }


             // This is kinda lazy on my part, however it easily prevents adding a help
             // Function to the recursion	
    private: System::Void ClearThumbs()
             {
                 this->prefabsTab->SuspendLayout();
                 this->splitContainer4->Panel1->SuspendLayout();
                 this->splitContainer4->SuspendLayout();


                 for(int i=0;i<thumbArray->Count;i++)
                 {					 
                     ((ThumbNailButton^)thumbArray->Item[i])->RemoveFromForm(splitContainer4->Panel2);
                 };

                 this->prefabsTab->ResumeLayout(false);
                 this->prefabsTab->PerformLayout();
                 this->splitContainer4->Panel1->ResumeLayout(false);
                 this->splitContainer4->ResumeLayout(false);
             };

    private: System::Void LoadThumbs(string path, 
                 int StartingPositionX,
                 int StartingPositionY,
                 int Width, int Height, 
                 int GridRight, 			 
                 System::Windows::Forms::SplitterPanel^ formAddTo)
             { 
                 ResetCurrentDirectory();

                 int NStartingPositionX=StartingPositionX;
                 int NStartingPositionY=StartingPositionY;
                 int NcurrentGrid=0;

                 bool foundThumbMath=false;

                 vector<string> dirs;
                 enumerateDirectories(path.c_str(),dirs,2);
                 for(int i=0;i<dirs.size();i++)
                 {
                     if(DirectoryHasXML(dirs[i]))
                     {
                         string XMLName = ToCppString(GetNameXML(dirs[i]));
                         foundThumbMath=false;

                         vector<string> bmpfiles;
                         enumerateFiles(dirs[i].c_str(),bmpfiles,0,".bmp");
                         int j=0;
                         for(j=0;j<bmpfiles.size();j++)
                         {
                             string bmpfile = bmpfiles[j].substr(0,bmpfiles[j].find_last_of("."));

                             if(bmpfile == XMLName)
                             {
                                 foundThumbMath=true;
                                 break;
                             };
                         };

                         if(GridRight==NcurrentGrid)
                         {
                             NStartingPositionX-= (Width*GridRight);
                             NStartingPositionY+= Height;
                             NcurrentGrid=0;
                         };

                         string loadFile;
                         if(foundThumbMath)
                         {
                             loadFile.append(dirs[i]);					
                             loadFile.append(bmpfiles[j]);
                         };

                         ThumbNailButton^ myButton = gcnew  ThumbNailButton(
                             foundThumbMath ?  gcnew String(loadFile.c_str()) : nullptr,						
                             gcnew String(XMLName.c_str()),
                             System::Drawing::Point(NStartingPositionX,
                             NStartingPositionY),
                             System::Drawing::Size(Width, Height),
                             formAddTo, this);	

                         string xmlLoadFile = dirs[i];
                         xmlLoadFile.append(XMLName);
                         xmlLoadFile.append(".xml");						 
                         myButton->fileName = STLToManaged(xmlLoadFile);

                         thumbArray->Add(myButton);					


                         NStartingPositionX+=Width;
                         NcurrentGrid++;

                     };//if has an xml
                 }//dir for
                 formAddTo->Update();
             };//main function
	private: System::Void toolStrip3_Click(System::Object^  sender, System::EventArgs^  e)
			 {
			 }

private: System::Void replacePrefabToolStripButton_Click(System::Object^  sender, System::EventArgs^  e)
		 {
			 if(!tvPrefabs->SelectedNode)
				 return;	
			 if(editor->m_SelectedActors.size()!=1)return;

			 Vector Position = editor->m_SelectedActors.at(0)->Location;
			 Matrix Rotation = editor->m_SelectedActors.at(0)->Rotation;


			 editor->m_SelectedActors.at(0)->MyWorld->RemoveActor(editor->m_SelectedActors.at(0));
			 delete editor->m_SelectedActors.at(0);			 
			 		 
			 if(tvPrefabs->SelectedNode->ImageIndex == DIR_IMAGE)
			 {
				 for(int i=0;i<thumbArray->Count;i++)	
				 {
					 if(	 ((ThumbNailButton^)thumbArray->Item[i])->IsSet() )
					 {
						 editor->MergeSceneUsingPosAndRotMat(
							 ToCppString(((ThumbNailButton^)thumbArray->Item[i])->fileName), 
							 Position, 
							 Rotation);
						 break;
					 }
				 };
			 }
			 //assume individual one is selected
			 else
			 {
				 editor->MergeSceneUsingPosAndRotMat(					 
					 ToCppString((String ^)tvPrefabs->SelectedNode->Tag), 
					 Position, 
					 Rotation);
			 };

			 editor->m_World->RegenerateOcclusionTree();
		 }

private: System::Void toolStripButton1_Click(System::Object^  sender, System::EventArgs^  e)
		 {
		 if(!selectedMat || !selectedMat->mat)
             return;
		 Material* newMat = new Material(selectedMat->mat->m_Name + ToStr(rand()%1000));
		 selectedMat->mat->CloneTo(newMat);

		 TreeNode^ node = gcnew TreeNode(gcnew String(newMat->m_Name.c_str()),MAT_IMAGE,MAT_IMAGE);
         tvMaterials->Nodes->Add(node);
         // Wrap material
         MatWrapper^ wrapper = gcnew MatWrapper();
         wrapper->mat = newMat;
         node->Tag = wrapper;
		 }

};
}