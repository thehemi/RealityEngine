//--------------------------------------------------------------------------------------
// Selection Dialog. Copyright Artificial Studios (c)2004 
//
// Author: Tim Johnson
//
// TODO: Update SelectionDialog selections based on m_SelectedActors when user
// changes their selection in the viewport
//
// TODO: Refresh when a new actor is deleted or added
//--------------------------------------------------------------------------------------
#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace StandaloneEditor
{
    /// <summary> 
    /// Summary for SelectForm
    ///
    /// WARNING: If you change the name of this class, you will need to change the 
    ///          'Resource File Name' property for the managed resource compiler tool 
    ///          associated with all .resx files this class depends on.  Otherwise,
    ///          the designers will not be able to interact properly with localized
    ///          resources associated with this form.
    /// </summary>
    public ref class SelectForm : public System::Windows::Forms::Form
    {
        // Data types
    public: bool showMesh, showLight, showEntity, showHidden, showFrozen;

    private: System::Windows::Forms::CheckBox^  checkHidden;
    private: System::Windows::Forms::CheckBox^  checkFrozen;

    public: String^ textFilter;

    public: 
        SelectForm(void)
        {
            textFilter = "";
            showMesh   = true;
            showLight  = true;
            showEntity = true;
            showHidden = true;
            showFrozen = true;
            InitializeComponent();
            //
            //TODO: Add the constructor code here
            //
            this->Closing += gcnew CancelEventHandler(this, &SelectForm::SelectForm_Closing);
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
    private: System::Windows::Forms::Button^  button1;


    private: System::Windows::Forms::DataGridViewTextBoxColumn^  dataGridViewTextBoxColumn4;
    private: System::Windows::Forms::DataGridViewTextBoxColumn^  dataGridViewTextBoxColumn5;
    private: System::Windows::Forms::DataGridViewTextBoxColumn^  dataGridViewTextBoxColumn6;
    public: System::Windows::Forms::DataGridView^  selectObjectsTable;
    private: System::Windows::Forms::Button^  selectAll;
    private: System::Windows::Forms::Button^  selectNone;
    private: System::Windows::Forms::Button^  invertSelection;
    private: System::Windows::Forms::GroupBox^  groupBox1;
    private: System::Windows::Forms::CheckBox^  checkPrefabs;
    private: System::Windows::Forms::CheckBox^  checkLights;
    private: System::Windows::Forms::CheckBox^  checkEntities;















    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle1 = gcnew System::Windows::Forms::DataGridViewCellStyle();
            this->button1 = gcnew System::Windows::Forms::Button();
            this->selectObjectsTable = gcnew System::Windows::Forms::DataGridView();
            this->dataGridViewTextBoxColumn4 = gcnew System::Windows::Forms::DataGridViewTextBoxColumn();
            this->dataGridViewTextBoxColumn5 = gcnew System::Windows::Forms::DataGridViewTextBoxColumn();
            this->dataGridViewTextBoxColumn6 = gcnew System::Windows::Forms::DataGridViewTextBoxColumn();
            this->selectAll = gcnew System::Windows::Forms::Button();
            this->selectNone = gcnew System::Windows::Forms::Button();
            this->invertSelection = gcnew System::Windows::Forms::Button();
            this->groupBox1 = gcnew System::Windows::Forms::GroupBox();
            this->checkFrozen = gcnew System::Windows::Forms::CheckBox();
            this->checkHidden = gcnew System::Windows::Forms::CheckBox();
            this->checkEntities = gcnew System::Windows::Forms::CheckBox();
            this->checkLights = gcnew System::Windows::Forms::CheckBox();
            this->checkPrefabs = gcnew System::Windows::Forms::CheckBox();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->selectObjectsTable))->BeginInit();
            this->groupBox1->SuspendLayout();
            this->SuspendLayout();
            this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->button1->AutoRelocate = true;
            this->button1->Location = System::Drawing::Point(408, 453);
            this->button1->Name = L"button1";
            this->button1->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
            this->button1->TabIndex = 0;
            this->button1->Text = L"Close";
            this->button1->Click += gcnew System::EventHandler(this, &SelectForm::button1_Click);
            this->selectObjectsTable->AllowUserToAddRows = false;
            this->selectObjectsTable->AllowUserToDeleteRows = false;
            this->selectObjectsTable->AllowUserToOrderColumns = true;
            this->selectObjectsTable->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->selectObjectsTable->AutoRelocate = true;
            this->selectObjectsTable->AutoSizeColumnHeadersEnabled = true;
            this->selectObjectsTable->AutoSizeRowHeadersMode = System::Windows::Forms::DataGridViewAutoSizeRowHeadersMode::AllRows;
            this->selectObjectsTable->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::HeaderAndColumnsDisplayedRows;
            this->selectObjectsTable->BackgroundColor = System::Drawing::SystemColors::Window;
            this->selectObjectsTable->ColumnHeadersHeight = 18;
            this->selectObjectsTable->Columns->Add(this->dataGridViewTextBoxColumn4);
            this->selectObjectsTable->Columns->Add(this->dataGridViewTextBoxColumn5);
            this->selectObjectsTable->Columns->Add(this->dataGridViewTextBoxColumn6);
            this->selectObjectsTable->Location = System::Drawing::Point(19, 18);
            this->selectObjectsTable->Name = L"selectObjectsTable";
            this->selectObjectsTable->ReadOnly = true;
            this->selectObjectsTable->RowHeadersVisible = false;
            this->selectObjectsTable->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
            this->selectObjectsTable->ShowEditingIcon = false;
            this->selectObjectsTable->Size = System::Drawing::Size(354, 458);
            this->selectObjectsTable->TabIndex = 2;
            this->selectObjectsTable->Click += gcnew System::EventHandler(this, &SelectForm::selectObjectsTable_Click);
            this->dataGridViewTextBoxColumn4->DefaultCellStyle = dataGridViewCellStyle1;
            this->dataGridViewTextBoxColumn4->HeaderText = L"Name";
            this->dataGridViewTextBoxColumn4->Name = L"CName";
            this->dataGridViewTextBoxColumn4->ReadOnly = true;
            this->dataGridViewTextBoxColumn4->Width = 150;
            this->dataGridViewTextBoxColumn5->DefaultCellStyle = dataGridViewCellStyle1;
            this->dataGridViewTextBoxColumn5->HeaderText = L"Type";
            this->dataGridViewTextBoxColumn5->Name = L"CType";
            this->dataGridViewTextBoxColumn5->ReadOnly = true;
            this->dataGridViewTextBoxColumn6->DefaultCellStyle = dataGridViewCellStyle1;
            this->dataGridViewTextBoxColumn6->HeaderText = L"Material";
            this->dataGridViewTextBoxColumn6->Name = L"CMaterial";
            this->dataGridViewTextBoxColumn6->ReadOnly = true;
            this->selectAll->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->selectAll->Location = System::Drawing::Point(392, 13);
            this->selectAll->Name = L"selectAll";
            this->selectAll->Size = System::Drawing::Size(91, 23);
            this->selectAll->TabIndex = 3;
            this->selectAll->Text = L"Select &All";
            this->selectAll->Click += gcnew System::EventHandler(this, &SelectForm::selectAll_Click);
            this->selectNone->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->selectNone->Location = System::Drawing::Point(392, 43);
            this->selectNone->Name = L"selectNone";
            this->selectNone->Size = System::Drawing::Size(91, 23);
            this->selectNone->TabIndex = 4;
            this->selectNone->Text = L"Select &None";
            this->selectNone->Click += gcnew System::EventHandler(this, &SelectForm::selectNone_Click);
            this->invertSelection->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->invertSelection->Location = System::Drawing::Point(392, 73);
            this->invertSelection->Name = L"invertSelection";
            this->invertSelection->Size = System::Drawing::Size(91, 23);
            this->invertSelection->TabIndex = 5;
            this->invertSelection->Text = L"&Invert Selection";
            this->invertSelection->Click += gcnew System::EventHandler(this, &SelectForm::invertSelection_Click);
            this->groupBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
            this->groupBox1->Controls->Add(this->checkFrozen);
            this->groupBox1->Controls->Add(this->checkHidden);
            this->groupBox1->Controls->Add(this->checkEntities);
            this->groupBox1->Controls->Add(this->checkLights);
            this->groupBox1->Controls->Add(this->checkPrefabs);
            this->groupBox1->Location = System::Drawing::Point(392, 103);
            this->groupBox1->Name = L"groupBox1";
            this->groupBox1->Size = System::Drawing::Size(91, 144);
            this->groupBox1->TabIndex = 6;
            this->groupBox1->TabStop = false;
            this->groupBox1->Text = L"List Types";
            this->checkFrozen->AutoSize = true;
            this->checkFrozen->Checked = true;
            this->checkFrozen->CheckState = System::Windows::Forms::CheckState::Checked;
            this->checkFrozen->Location = System::Drawing::Point(16, 68);
            this->checkFrozen->Name = L"checkFrozen";
            this->checkFrozen->Size = System::Drawing::Size(54, 17);
            this->checkFrozen->TabIndex = 4;
            this->checkFrozen->Text = L"Frozen";
            this->checkFrozen->CheckedChanged += gcnew System::EventHandler(this, &SelectForm::checkFrozen_CheckedChanged);
            this->checkHidden->AutoSize = true;
            this->checkHidden->Checked = true;
            this->checkHidden->CheckState = System::Windows::Forms::CheckState::Checked;
            this->checkHidden->Location = System::Drawing::Point(16, 44);
            this->checkHidden->Name = L"checkHidden";
            this->checkHidden->Size = System::Drawing::Size(56, 17);
            this->checkHidden->TabIndex = 3;
            this->checkHidden->Text = L"Hidden";
            this->checkHidden->CheckedChanged += gcnew System::EventHandler(this, &SelectForm::checkHidden_CheckedChanged);
            this->checkEntities->AutoSize = true;
            this->checkEntities->Checked = true;
            this->checkEntities->CheckState = System::Windows::Forms::CheckState::Checked;
            this->checkEntities->Location = System::Drawing::Point(7, 116);
            this->checkEntities->Name = L"checkEntities";
            this->checkEntities->Size = System::Drawing::Size(56, 17);
            this->checkEntities->TabIndex = 2;
            this->checkEntities->Text = L"Entities";
            this->checkEntities->CheckedChanged += gcnew System::EventHandler(this, &SelectForm::checkEntities_CheckedChanged);
            this->checkLights->AutoSize = true;
            this->checkLights->Checked = true;
            this->checkLights->CheckState = System::Windows::Forms::CheckState::Checked;
            this->checkLights->Location = System::Drawing::Point(7, 92);
            this->checkLights->Name = L"checkLights";
            this->checkLights->Size = System::Drawing::Size(50, 17);
            this->checkLights->TabIndex = 1;
            this->checkLights->Text = L"Lights";
            this->checkLights->CheckedChanged += gcnew System::EventHandler(this, &SelectForm::checkLights_CheckedChanged);
            this->checkPrefabs->AutoSize = true;
            this->checkPrefabs->Checked = true;
            this->checkPrefabs->CheckState = System::Windows::Forms::CheckState::Checked;
            this->checkPrefabs->Location = System::Drawing::Point(7, 20);
            this->checkPrefabs->Name = L"checkPrefabs";
            this->checkPrefabs->Size = System::Drawing::Size(58, 17);
            this->checkPrefabs->TabIndex = 0;
            this->checkPrefabs->Text = L"Prefabs";
            this->checkPrefabs->CheckedChanged += gcnew System::EventHandler(this, &SelectForm::checkPrefabs_CheckedChanged);
            this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
            this->ClientSize = System::Drawing::Size(495, 497);
            this->Controls->Add(this->groupBox1);
            this->Controls->Add(this->invertSelection);
            this->Controls->Add(this->selectNone);
            this->Controls->Add(this->selectAll);
            this->Controls->Add(this->selectObjectsTable);
            this->Controls->Add(this->button1);
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"SelectForm";
            this->ShowInTaskbar = false;
            this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Show;
            this->Text = L"Select Object(s)";
            this->TopMost = true;
            this->Load += gcnew System::EventHandler(this, &SelectForm::SelectForm_Load);
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->selectObjectsTable))->EndInit();
            this->groupBox1->ResumeLayout(false);
            this->groupBox1->PerformLayout();
            this->ResumeLayout(false);

        }		
#pragma endregion
    private: System::Void SelectForm_Load(System::Object^  sender, System::EventArgs^  e)
             {
             }

    private: System::Void SelectForm_Closing(System::Object^  sender, CancelEventArgs^ e)
             {
                 e->Cancel = true;
                 this->Hide();
             }

    public: void Fill()
            {
                showEntity = checkEntities->Checked;
                showLight  = checkLights->Checked;
                showMesh   = checkPrefabs->Checked;
                showHidden = checkHidden->Checked;
                showFrozen = checkFrozen->Checked;

                selectObjectsTable->Rows->Clear();

                string strFilter = ToCppString(textFilter);

                // Add world actors/meshes

                for(int i=0;i<Editor::Instance()->m_World->m_Actors.size();i++)
                {
                    Actor* a = Editor::Instance()->m_World->m_Actors[i];

                    // Run text filter
                    if(textFilter->Length && a->m_Name.find(strFilter) != -1)
                        continue;

                    string type, matName;

                    //
                    // Mesh type
                    //
                    if(a->MyModel && showMesh){

                        if(!(a->IsFrozen && !showFrozen || a->IsHidden && !showHidden))
                        {
                            // Insert row with name, type, matname

                            // Gather material name(s)
                           
                            if(a->MyModel->m_pFrameRoot)
                            {
                                vector<Material*> mats;
                                a->MyModel->m_pFrameRoot->FindMaterials(mats);
                                for(int j=0;j<mats.size();j++)
                                {
                                    matName += mats[j]->m_Name.c_str();
                                    if(j<mats.size()-1)
                                        matName += ", ";
                                }
                            }

                            type += "Prefab";
                            if(a->IsHidden)type += " (Hidden)";
                            if(a->IsFrozen)type += " (Frozen)";						
                        }
                    }
                    //
                    // Light type
                    //
                    if(showLight && a->IsLight())
                    {
                        if(type.length()) type += ", ";
                        type += "Light";
                    }
                    //
                    // Script/Entity type. Only add this if it has a script, or it didn't fall into previous categories
                    //
                    if((a->script.classname.length() || type.length()==0) && showEntity && a->script.classname.find("Prefab") == -1 && a->script.classname.find("StaticMesh") == -1)
                    {
                        if(type.length()) type += ", ";
                        type += "Entity/Script";
                    }

                    // Add listing to table
                    if(type.length())
                        selectObjectsTable->Rows->Add(gcnew String(a->m_Name.c_str()),gcnew String(type.c_str()),gcnew String(matName.c_str()));
                }
            }

    private: System::Void checkEntities_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
             {


                 Fill();
             }

    private: System::Void checkPrefabs_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 if(!checkPrefabs->Checked)
                 {
                     checkHidden->Disable();
                     checkFrozen->Disable();
                 } else
                 {
                     checkHidden->Enable();
                     checkFrozen->Enable();
                 };

                 Fill();
             }

    private: System::Void checkLights_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 Fill();
             }

    private: System::Void selectAll_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 selectObjectsTable->SelectAll();
                 UpdateSelections();
             }

    private: System::Void selectNone_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 selectObjectsTable->ClearSelection();
                 UpdateSelections();
             }

    private: System::Void invertSelection_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 for(int i=0;i<selectObjectsTable->Rows->Count;i++)
                 {
                     selectObjectsTable->Rows->Item[i]->Selected = !selectObjectsTable->Rows->Item[i]->Selected;
                 }
                 UpdateSelections();
             }

    private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 Hide();
             }

    private: System::Void selectObjectsTable_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 UpdateSelections();
             }

    private: System::Void UpdateSelections()
             {
                 Editor::Instance()->UnSelectAll();
                 for(int i=0;i<selectObjectsTable->SelectedRows->Count;i++)
                 {
                     string name = ToCppString(selectObjectsTable->SelectedRows->Item[i]->Cells[0]->Value->ToString());
                     Actor* a = Editor::Instance()->m_World->FindActor(name);
                     if(a)
                     {
                         a->IsSelected = true;
                         Editor::Instance()->m_SelectedActors.push_back(a);
                         Editor::Instance()->m_bSelectionChanged = true;
                     }
                     else
                     {
                         // Actor must have been deleted, remove it
                         selectObjectsTable->SelectedRows->RemoveAt(i);
                         i--;
                     }
                 }
             }

    private: System::Void checkHidden_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 Fill();
             }

    private: System::Void checkFrozen_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
             {
                 Fill();
             }

    };
}