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
	/// Summary for MeshSettings
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class MeshSettings : public System::Windows::Forms::Form
	{
	public: 
		MeshSettings(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
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


    private: System::Windows::Forms::Label^  label1;
    public: System::Windows::Forms::NumericUpDown^  minCosCrease;
    public: System::Windows::Forms::CheckBox^  fixCylindrical;
    public: System::Windows::Forms::CheckBox^  respectSplits;





    private: System::Windows::Forms::Button^  buttonOK;
    public: System::Windows::Forms::NumericUpDown^  normalWeighting;

    private: System::Windows::Forms::Label^  label2;






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
            this->minCosCrease = gcnew System::Windows::Forms::NumericUpDown();
            this->label1 = gcnew System::Windows::Forms::Label();
            this->fixCylindrical = gcnew System::Windows::Forms::CheckBox();
            this->respectSplits = gcnew System::Windows::Forms::CheckBox();
            this->buttonOK = gcnew System::Windows::Forms::Button();
            this->normalWeighting = gcnew System::Windows::Forms::NumericUpDown();
            this->label2 = gcnew System::Windows::Forms::Label();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->minCosCrease))->BeginInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->normalWeighting))->BeginInit();
            this->SuspendLayout();
            this->minCosCrease->Location = System::Drawing::Point(147, 13);
            this->minCosCrease->Margin = System::Windows::Forms::Padding(3, 3, 3, 1);
            this->minCosCrease->Maximum = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {180, 0, 0, 0});
            this->minCosCrease->Name = L"minCosCrease";
            this->minCosCrease->Size = System::Drawing::Size(44, 20);
            this->minCosCrease->TabIndex = 0;
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(13, 13);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(133, 14);
            this->label1->TabIndex = 2;
            this->label1->Text = L"Min crease angle (0-180):";
            this->fixCylindrical->AutoSize = true;
            this->fixCylindrical->Location = System::Drawing::Point(13, 68);
            this->fixCylindrical->Name = L"fixCylindrical";
            this->fixCylindrical->Size = System::Drawing::Size(130, 17);
            this->fixCylindrical->TabIndex = 4;
            this->fixCylindrical->Text = L"Fix cylindrical wrapping";
            this->respectSplits->AutoSize = true;
            this->respectSplits->Location = System::Drawing::Point(13, 92);
            this->respectSplits->Name = L"respectSplits";
            this->respectSplits->Size = System::Drawing::Size(126, 17);
            this->respectSplits->TabIndex = 5;
            this->respectSplits->Text = L"Respect existing splits";
            this->buttonOK->Location = System::Drawing::Point(245, 92);
            this->buttonOK->Name = L"buttonOK";
            this->buttonOK->TabIndex = 6;
            this->buttonOK->Text = L"OK";
            this->buttonOK->Click += gcnew System::EventHandler(this, &MeshSettings::buttonOK_Click);
            this->normalWeighting->ImeMode = System::Windows::Forms::ImeMode::On;
            this->normalWeighting->Increment = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 131072});
            this->normalWeighting->Location = System::Drawing::Point(147, 41);
            this->normalWeighting->Name = L"normalWeighting";
            this->normalWeighting->Size = System::Drawing::Size(44, 20);
            this->normalWeighting->TabIndex = 7;
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(13, 41);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(127, 14);
            this->label2->TabIndex = 8;
            this->label2->Text = L"Weight normals by area:";
            this->label2->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
            this->ClientSize = System::Drawing::Size(332, 125);
            this->Controls->Add(this->label2);
            this->Controls->Add(this->normalWeighting);
            this->Controls->Add(this->buttonOK);
            this->Controls->Add(this->respectSplits);
            this->Controls->Add(this->fixCylindrical);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->minCosCrease);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
            this->Name = L"MeshSettings";
            this->Text = L"MeshSettings";
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->minCosCrease))->EndInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->normalWeighting))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }		
#pragma endregion
    private: System::Void buttonOK_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 Hide();
             }

};
}