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
	/// Summary for SplitForm
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SplitForm : public System::Windows::Forms::Form
	{
	public: 
		SplitForm(void)
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
    private: System::Windows::Forms::Label^  label2;
    private: System::Windows::Forms::Label^  label3;
    public: System::Windows::Forms::NumericUpDown^  xSegs;



    public: System::Windows::Forms::NumericUpDown^  ySegs;

    public: System::Windows::Forms::NumericUpDown^  zSegs;


    private: System::Windows::Forms::Button^  buttonOK;


    private: System::Windows::Forms::Label^  label4;
    private: System::Windows::Forms::Label^  label5;
    private: System::Windows::Forms::Label^  label6;
    public: System::Windows::Forms::NumericUpDown^  xOff;
    public: System::Windows::Forms::NumericUpDown^  zOff;
    public: System::Windows::Forms::NumericUpDown^  yOff;



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
            this->xSegs = gcnew System::Windows::Forms::NumericUpDown();
            this->label1 = gcnew System::Windows::Forms::Label();
            this->label2 = gcnew System::Windows::Forms::Label();
            this->label3 = gcnew System::Windows::Forms::Label();
            this->ySegs = gcnew System::Windows::Forms::NumericUpDown();
            this->zSegs = gcnew System::Windows::Forms::NumericUpDown();
            this->buttonOK = gcnew System::Windows::Forms::Button();
            this->zOff = gcnew System::Windows::Forms::NumericUpDown();
            this->yOff = gcnew System::Windows::Forms::NumericUpDown();
            this->label4 = gcnew System::Windows::Forms::Label();
            this->label5 = gcnew System::Windows::Forms::Label();
            this->label6 = gcnew System::Windows::Forms::Label();
            this->xOff = gcnew System::Windows::Forms::NumericUpDown();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->xSegs))->BeginInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ySegs))->BeginInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->zSegs))->BeginInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->zOff))->BeginInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->yOff))->BeginInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->xOff))->BeginInit();
            this->SuspendLayout();
            // 
            // xSegs
            // 
            this->xSegs->Location = System::Drawing::Point(89, 13);
            this->xSegs->Minimum = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 0});
            this->xSegs->Name = L"xSegs";
            this->xSegs->Size = System::Drawing::Size(41, 20);
            this->xSegs->TabIndex = 0;
            this->xSegs->Value = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {3, 0, 0, 0});
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(13, 13);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(69, 14);
            this->label1->TabIndex = 1;
            this->label1->Text = L"X Segments:";
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(13, 40);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(69, 14);
            this->label2->TabIndex = 2;
            this->label2->Text = L"Y Segments:";
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(13, 67);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(68, 14);
            this->label3->TabIndex = 3;
            this->label3->Text = L"Z Segments:";
            // 
            // ySegs
            // 
            this->ySegs->Location = System::Drawing::Point(89, 40);
            this->ySegs->Minimum = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 0});
            this->ySegs->Name = L"ySegs";
            this->ySegs->Size = System::Drawing::Size(41, 20);
            this->ySegs->TabIndex = 4;
            this->ySegs->Value = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 0});
            this->ySegs->ValueChanged += gcnew System::EventHandler(this, &SplitForm::ySegs_ValueChanged);
            // 
            // zSegs
            // 
            this->zSegs->Location = System::Drawing::Point(89, 67);
            this->zSegs->Minimum = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 0});
            this->zSegs->Name = L"zSegs";
            this->zSegs->Size = System::Drawing::Size(41, 20);
            this->zSegs->TabIndex = 5;
            this->zSegs->Value = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {3, 0, 0, 0});
            // 
            // buttonOK
            // 
            this->buttonOK->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->buttonOK->Location = System::Drawing::Point(305, 68);
            this->buttonOK->Name = L"buttonOK";
            this->buttonOK->TabIndex = 7;
            this->buttonOK->Text = L"OK";
            this->buttonOK->Click += gcnew System::EventHandler(this, &SplitForm::buttonExport_Click);
            // 
            // zOff
            // 
            this->zOff->Increment = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 65536});
            this->zOff->Location = System::Drawing::Point(228, 68);
            this->zOff->Maximum = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {10000, 0, 0, 0});
            this->zOff->Name = L"zOff";
            this->zOff->Size = System::Drawing::Size(41, 20);
            this->zOff->TabIndex = 13;
            // 
            // yOff
            // 
            this->yOff->Increment = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 65536});
            this->yOff->Location = System::Drawing::Point(228, 41);
            this->yOff->Maximum = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {10000, 0, 0, 0});
            this->yOff->Name = L"yOff";
            this->yOff->Size = System::Drawing::Size(41, 20);
            this->yOff->TabIndex = 12;
            this->yOff->ValueChanged += gcnew System::EventHandler(this, &SplitForm::ySegs_ValueChanged);
            // 
            // label4
            // 
            this->label4->AutoSize = true;
            this->label4->Location = System::Drawing::Point(152, 68);
            this->label4->Name = L"label4";
            this->label4->Size = System::Drawing::Size(47, 14);
            this->label4->TabIndex = 11;
            this->label4->Text = L"Z Offset:";
            // 
            // label5
            // 
            this->label5->AutoSize = true;
            this->label5->Location = System::Drawing::Point(152, 41);
            this->label5->Name = L"label5";
            this->label5->Size = System::Drawing::Size(48, 14);
            this->label5->TabIndex = 10;
            this->label5->Text = L"Y Offset:";
            // 
            // label6
            // 
            this->label6->AutoSize = true;
            this->label6->Location = System::Drawing::Point(152, 14);
            this->label6->Name = L"label6";
            this->label6->Size = System::Drawing::Size(48, 14);
            this->label6->TabIndex = 9;
            this->label6->Text = L"X Offset:";
            // 
            // xOff
            // 
            this->xOff->Increment = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {1, 0, 0, 65536});
            this->xOff->Location = System::Drawing::Point(228, 14);
            this->xOff->Maximum = System::Decimal(gcnew stdcli::language::array<System::Int32>(4) {10000, 0, 0, 0});
            this->xOff->Name = L"xOff";
            this->xOff->Size = System::Drawing::Size(41, 20);
            this->xOff->TabIndex = 8;
            // 
            // SplitForm
            // 
            this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
            this->ClientSize = System::Drawing::Size(390, 103);
            this->Controls->Add(this->zOff);
            this->Controls->Add(this->yOff);
            this->Controls->Add(this->label4);
            this->Controls->Add(this->label5);
            this->Controls->Add(this->label6);
            this->Controls->Add(this->xOff);
            this->Controls->Add(this->buttonOK);
            this->Controls->Add(this->zSegs);
            this->Controls->Add(this->ySegs);
            this->Controls->Add(this->label3);
            this->Controls->Add(this->label2);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->xSegs);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
            this->Name = L"SplitForm";
            this->Text = L"Split Mesh";
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->xSegs))->EndInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ySegs))->EndInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->zSegs))->EndInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->zOff))->EndInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->yOff))->EndInit();
            (stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->xOff))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }		
#pragma endregion
    private: System::Void buttonExport_Click(System::Object^  sender, System::EventArgs^  e)
             {
                 Hide();
             }

private: System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e)
         {
             Hide();
         }

private: System::Void ySegs_ValueChanged(System::Object^  sender, System::EventArgs^  e)
         {
         }

};
}