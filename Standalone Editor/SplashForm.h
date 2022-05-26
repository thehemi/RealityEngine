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
	/// Summary for SplashForm
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class SplashForm : public System::Windows::Forms::Form
	{
	public: 
		SplashForm(void)
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

	private: System::Windows::Forms::RaftingContainer^  leftRaftingContainer;
	private: System::Windows::Forms::RaftingContainer^  leftRaftingContainer1;

	private: System::Windows::Forms::RaftingContainer^  bottomRaftingContainer;




	private: System::Windows::Forms::RaftingContainer^  dsfdf;
	private: System::Windows::Forms::PictureBox^  pictureBox1;







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
			System::ComponentModel::ComponentResourceManager^  resources = gcnew System::ComponentModel::ComponentResourceManager(typeid<SplashForm >);
			this->leftRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
			this->leftRaftingContainer1 = gcnew System::Windows::Forms::RaftingContainer();
			this->dsfdf = gcnew System::Windows::Forms::RaftingContainer();
			this->bottomRaftingContainer = gcnew System::Windows::Forms::RaftingContainer();
			this->pictureBox1 = gcnew System::Windows::Forms::PictureBox();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer))->BeginInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer1))->BeginInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dsfdf))->BeginInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bottomRaftingContainer))->BeginInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			this->leftRaftingContainer->Dock = System::Windows::Forms::DockStyle::Left;
			this->leftRaftingContainer->Margin = System::Windows::Forms::Padding(0, 0, 1, 0);
			this->leftRaftingContainer->Name = L"leftRaftingContainer";
			this->leftRaftingContainer->UseWaitCursor = true;
			this->leftRaftingContainer1->Dock = System::Windows::Forms::DockStyle::Left;
			this->leftRaftingContainer1->Name = L"leftRaftingContainer1";
			this->leftRaftingContainer1->UseWaitCursor = true;
			this->dsfdf->Dock = System::Windows::Forms::DockStyle::Top;
			this->dsfdf->Name = L"dsfdf";
			this->dsfdf->UseWaitCursor = true;
			this->bottomRaftingContainer->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->bottomRaftingContainer->Name = L"bottomRaftingContainer";
			this->bottomRaftingContainer->UseWaitCursor = true;
			this->pictureBox1->Image = (stdcli::language::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox1.Image")));
			this->pictureBox1->Location = System::Drawing::Point(9, 10);
			this->pictureBox1->Margin = System::Windows::Forms::Padding(0, 3, 3, 3);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(760, 381);
			this->pictureBox1->TabIndex = 14;
			this->pictureBox1->TabStop = false;
			this->pictureBox1->UseWaitCursor = true;
			this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
			this->BackColor = System::Drawing::SystemColors::ControlDarkDark;
			this->ClientSize = System::Drawing::Size(749, 369);
			this->ControlBox = false;
			this->Controls->Add(this->pictureBox1);
			this->Controls->Add(this->leftRaftingContainer);
			this->Controls->Add(this->leftRaftingContainer1);
			this->Controls->Add(this->dsfdf);
			this->Controls->Add(this->bottomRaftingContainer);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SplashForm";
			this->ShowInTaskbar = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"SplashForm";
			this->UseWaitCursor = true;
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer))->EndInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->leftRaftingContainer1))->EndInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dsfdf))->EndInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bottomRaftingContainer))->EndInit();
			(stdcli::language::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}		
#pragma endregion
	private: System::Void pictureBox1_Click(System::Object^  sender, System::EventArgs^  e)
			 {
			 }

private: System::Void pictureBox1_Click_1(System::Object^  sender, System::EventArgs^  e)
		 {
		 }

};
}