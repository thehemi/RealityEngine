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
	/// Summary for OBJExportForm
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class OBJExportForm : public System::Windows::Forms::Form
	{
	public: 
		bool bExport;

		OBJExportForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			bExport = false;
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
	public: System::Windows::Forms::TextBox^  unitScale;


	private: System::Windows::Forms::GroupBox^  groupBox1;
	public: System::Windows::Forms::RadioButton^  radioMax;
	public: System::Windows::Forms::RadioButton^  radioMaya;
	public: System::Windows::Forms::RadioButton^  radioD3D;
	private: System::Windows::Forms::Button^  buttonExport;
    public: System::Windows::Forms::CheckBox^  stripHierarchy;


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
            this->radioMax = gcnew System::Windows::Forms::RadioButton();
            this->radioMaya = gcnew System::Windows::Forms::RadioButton();
            this->radioD3D = gcnew System::Windows::Forms::RadioButton();
            this->unitScale = gcnew System::Windows::Forms::TextBox();
            this->label1 = gcnew System::Windows::Forms::Label();
            this->groupBox1 = gcnew System::Windows::Forms::GroupBox();
            this->buttonExport = gcnew System::Windows::Forms::Button();
            this->stripHierarchy = gcnew System::Windows::Forms::CheckBox();
            this->groupBox1->SuspendLayout();
            this->SuspendLayout();
            // 
            // radioMax
            // 
            this->radioMax->AutoSize = true;
            this->radioMax->Location = System::Drawing::Point(7, 20);
            this->radioMax->Name = L"radioMax";
            this->radioMax->Size = System::Drawing::Size(74, 17);
            this->radioMax->TabIndex = 0;
            this->radioMax->Text = L"Max (Z-Up)";
            this->radioMax->CheckedChanged += gcnew System::EventHandler(this, &OBJExportForm::radioMax_CheckedChanged);
            // 
            // radioMaya
            // 
            this->radioMaya->AutoSize = true;
            this->radioMaya->Location = System::Drawing::Point(7, 44);
            this->radioMaya->Name = L"radioMaya";
            this->radioMaya->Size = System::Drawing::Size(99, 17);
            this->radioMaya->TabIndex = 1;
            this->radioMaya->Text = L"Maya (RH Y-Up)";
            this->radioMaya->CheckedChanged += gcnew System::EventHandler(this, &OBJExportForm::radioMaya_CheckedChanged);
            // 
            // radioD3D
            // 
            this->radioD3D->AutoSize = true;
            this->radioD3D->Location = System::Drawing::Point(7, 68);
            this->radioD3D->Name = L"radioD3D";
            this->radioD3D->Size = System::Drawing::Size(96, 17);
            this->radioD3D->TabIndex = 2;
            this->radioD3D->Text = L"Reality/DirectX ";
            this->radioD3D->CheckedChanged += gcnew System::EventHandler(this, &OBJExportForm::radioD3D_CheckedChanged);
            // 
            // unitScale
            // 
            this->unitScale->Location = System::Drawing::Point(318, 9);
            this->unitScale->Name = L"unitScale";
            this->unitScale->Size = System::Drawing::Size(55, 20);
            this->unitScale->TabIndex = 3;
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(177, 12);
            this->label1->Name = L"label1";
            this->label1->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
            this->label1->Size = System::Drawing::Size(134, 14);
            this->label1->TabIndex = 4;
            this->label1->Text = L"Unit Scale (From Meters):";
            // 
            // groupBox1
            // 
            this->groupBox1->Controls->Add(this->radioD3D);
            this->groupBox1->Controls->Add(this->radioMaya);
            this->groupBox1->Controls->Add(this->radioMax);
            this->groupBox1->Location = System::Drawing::Point(5, 0);
            this->groupBox1->Name = L"groupBox1";
            this->groupBox1->Size = System::Drawing::Size(120, 90);
            this->groupBox1->TabIndex = 5;
            this->groupBox1->TabStop = false;
            this->groupBox1->Text = L"Coordinate System";
            // 
            // buttonExport
            // 
            this->buttonExport->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->buttonExport->Location = System::Drawing::Point(298, 68);
            this->buttonExport->Name = L"buttonExport";
            this->buttonExport->TabIndex = 6;
            this->buttonExport->Text = L"OK";
            this->buttonExport->Click += gcnew System::EventHandler(this, &OBJExportForm::buttonExport_Click);
            // 
            // stripHierarchy
            // 
            this->stripHierarchy->AutoSize = true;
            this->stripHierarchy->Location = System::Drawing::Point(132, 36);
            this->stripHierarchy->Name = L"stripHierarchy";
            this->stripHierarchy->Size = System::Drawing::Size(246, 17);
            this->stripHierarchy->TabIndex = 7;
            this->stripHierarchy->Text = L"Strip Hierarchy (Recommended for scene items)";
            // 
            // OBJExportForm
            // 
            this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
            this->ClientSize = System::Drawing::Size(382, 101);
            this->Controls->Add(this->stripHierarchy);
            this->Controls->Add(this->buttonExport);
            this->Controls->Add(this->groupBox1);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->unitScale);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->Name = L"OBJExportForm";
            this->Text = L"Import/Export Options";
            this->groupBox1->ResumeLayout(false);
            this->groupBox1->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }		
#pragma endregion
	private: System::Void buttonExport_Click(System::Object^  sender, System::EventArgs^  e)
			 {
				 Hide();
			 }

private: System::Void radioMax_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		 {
			 if(radioMax->Checked)
			 {
				 if(bExport)
					 unitScale->Text = "39.3700787"; // Meters to inches
				 else
					 unitScale->Text = "0.0254";     // Inches to meters
			 }
		 }

private: System::Void radioMaya_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		 {
			 if(radioMaya->Checked)
			 {
				 if(bExport)
					 unitScale->Text = "100.0"; // Meters to centimeters
				 else
					 unitScale->Text = "0.01";  // Centimeters to meters
			 }
		 }

private: System::Void radioD3D_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		 {
			 if(radioD3D->Checked)
			 {
				 if(bExport)
					 unitScale->Text = "1";  // Meters to meters
				 else
					 unitScale->Text = "1";  // Meters to meters
			 }
		 }

};
}