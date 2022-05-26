#region Using directives

using System;
using System.Collections;
using System.Text;

using System.ComponentModel;
using System.ComponentModel.Design;
using System.ComponentModel.Design.Serialization;

using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Design;

using System.Windows.Forms;
#endregion

namespace ScriptingSystem
{
	[ToolboxItemFilter("ArtificialStudios.GuiControls", ToolboxItemFilterType.Prevent)]
	internal class DesignerView : UserControl
	{
		private MGUIDialog m_dialog;
		private DialogDesigner m_designer;
		public ISelectionService m_selectionSvc;
		public IDesignerHost m_host;
		private IToolboxService m_toolboxSvc;


        private System.Windows.Forms.Panel panel1;
        public DialogEditor dialogPanel;
        public System.Windows.Forms.Panel captionPanel;

		private object m_hitObject;
		private MGUIControl m_hitControl;
		private DragObject m_dragObject;
		private ToolboxItem m_toolboxItem;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.panel1 = new System.Windows.Forms.Panel();
            this.dialogPanel = new ScriptingSystem.DialogEditor();
            this.captionPanel = new System.Windows.Forms.Panel();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
// 
// panel1
// 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.dialogPanel);
            this.panel1.Controls.Add(this.captionPanel);
            this.panel1.Location = new System.Drawing.Point(4, 4);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(469, 361);
            this.panel1.TabIndex = 0;
// 
// dialogPanel
// 
            this.dialogPanel.AllowDrop = true;
            this.dialogPanel.BackColor = System.Drawing.Color.Black;
            this.dialogPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dialogPanel.Location = new System.Drawing.Point(0, 19);
            this.dialogPanel.Name = "dialogPanel";
            this.dialogPanel.Size = new System.Drawing.Size(467, 340);
            this.dialogPanel.TabIndex = 3;
            this.dialogPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.dialogPanel_Paint);
            this.dialogPanel.DragOver += new System.Windows.Forms.DragEventHandler(this.dialogPanel_DragOver);
            this.dialogPanel.MouseMove += new System.Windows.Forms.MouseEventHandler(this.dialogPanel_MouseMove);
            this.dialogPanel.DragDrop += new System.Windows.Forms.DragEventHandler(this.dialogPanel_DragDrop);
            this.dialogPanel.DragLeave += new System.EventHandler(this.dialogPanel_DragLeave);
            this.dialogPanel.DragEnter += new System.Windows.Forms.DragEventHandler(this.dialogPanel_DragEnter);
            this.dialogPanel.MouseUp += new System.Windows.Forms.MouseEventHandler(this.dialogPanel_MouseUp);
            this.dialogPanel.GiveFeedback += new System.Windows.Forms.GiveFeedbackEventHandler(this.dialogPanel_GiveFeedback);
            this.dialogPanel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.dialogPanel_MouseDown);
// 
// captionPanel
// 
            this.captionPanel.BackColor = System.Drawing.Color.Black;
            this.captionPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.captionPanel.Location = new System.Drawing.Point(0, 0);
            this.captionPanel.Name = "captionPanel";
            this.captionPanel.Size = new System.Drawing.Size(467, 19);
            this.captionPanel.TabIndex = 2;
            this.captionPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.captionPanel_Paint);
// 
// DesignerView
// 
            this.AutoScroll = true;
            this.Controls.Add(this.panel1);
            this.Name = "DesignerView";
            this.Size = new System.Drawing.Size(487, 397);
            this.Load += new System.EventHandler(this.DesignerView_Load);
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

		#endregion

		private IDesignerHost DesignerHost
		{
			get
			{
				if (m_host == null && m_designer != null)
				{
					m_host = (IDesignerHost)m_designer.GetServiceP(typeof(IDesignerHost));
				}
				return m_host;
			}
		}

		private ISelectionService SelectionService
		{
			get
			{
				if (m_selectionSvc == null && m_designer != null)
				{
					m_selectionSvc = (ISelectionService)m_designer.GetServiceP(typeof(ISelectionService));
				}
				return m_selectionSvc;
			}
		}

		private IToolboxService ToolboxService
		{
			get
			{
				if (m_toolboxSvc == null && m_designer != null)
				{
					m_toolboxSvc = (IToolboxService)m_designer.GetServiceP(typeof(IToolboxService));
				}

				return m_toolboxSvc;
			}
		}

		internal DesignerView(MGUIDialog dialog,DialogDesigner designer)
		{
			m_designer = designer;
			m_dialog = dialog;
			InitializeComponent();
		}

		public void InvokeToolboxItem(ToolboxItem item, int x, int y)
		{
			IDesignerHost host = DesignerHost;
			if (host != null)
			{
				using (DesignerTransaction trans = host.CreateTransaction("Creating " + item.DisplayName))
				{
					IComponent[] newComponents = item.CreateComponents(host);
					Rectangle bounds = new Rectangle();
					bounds.X = x - 50;
					bounds.Y = y - 50;
					bounds.Width = 100;
					bounds.Height = 100;

					MGUIDialog sc = host.RootComponent as MGUIDialog;
                    IComponentChangeService cs = (IComponentChangeService)m_designer.GetServiceP(typeof(IComponentChangeService));
                    MemberDescriptor desc = TypeDescriptor.GetProperties(host.RootComponent)["Controls"];
                    cs.OnComponentChanging(host.RootComponent, desc);
                    foreach (IComponent comp in newComponents)
					{
						if (!(comp is MGUIControl))
						{
							MessageBox.Show("Not a GUI Control\nType:" + comp.GetType().FullName);
						}
						if (sc != null && comp is MGUIControl)
						{
                            
                            sc.Controls.Add((MGUIControl)comp);
                        }

						PropertyDescriptor boundsProp = TypeDescriptor.GetProperties(comp)["BoundingBox"];
						if (boundsProp != null && boundsProp.PropertyType == typeof(Rectangle))
						{
							boundsProp.SetValue(comp, bounds);
							bounds.X += 10;
							bounds.Y += 10;
						}
					}

					if (sc.Controls.Count == 1)
					{
						dialogPanel.Invalidate();
					}

                    cs.OnComponentChanged(host.RootComponent, desc, null, null);
                    trans.Commit();

					IToolboxService ts = ToolboxService;
					if (ts != null)
					{
						ts.SelectedToolboxItemUsed();
					}

					ISelectionService ss = SelectionService;
					if (ss != null)
					{
						ss.SetSelectedComponents(newComponents, SelectionTypes.Replace);
					}
				}
			}
		}

		private void dialogPanel_Paint(object sender, PaintEventArgs e)
		{
			if (m_designer.m_editorDialog.EnableCaption == true)
			{
				if (!captionPanel.Visible)
					captionPanel.Visible=true;
			}
			else
			{
				if (captionPanel.Visible)
					captionPanel.Visible=false;
			}
            if (panel1.Size.Width != m_designer.m_editorDialog.Size.Width  ||
                panel1.Size.Height != m_designer.m_editorDialog.Size.Height + captionPanel.Height)
            {
                panel1.Size = m_designer.m_editorDialog.Size;
                panel1.Height += captionPanel.Height;
            }

            Graphics g = e.Graphics;
			LinearGradientBrush gradient = new LinearGradientBrush(
				new Point(0, 0), new Point(dialogPanel.ClientRectangle.Width, dialogPanel.ClientRectangle.Height),
				GUIProperties.backColorTopLeft, GUIProperties.backColorBottomRight);
			g.FillRectangle(gradient, e.ClipRectangle);

			if (m_designer.m_editorDialog.Controls.Count > 0)
			{
			
				if (m_designer.m_editorDialog.DrawGrid)
				{
					Pen p = new Pen(new HatchBrush(HatchStyle.DottedGrid,Color.DarkGray),50);
					for (int x=1;x < dialogPanel.Width ;x+=m_designer.m_editorDialog.GridSize.Width)
						g.DrawLine(Pens.DarkGray,x,0,x,dialogPanel.Height);
					for (int y=1;y < dialogPanel.Height ;y+=m_designer.m_editorDialog.GridSize.Height )
						g.DrawLine(Pens.DarkGray,0,y,dialogPanel.Width,y);
				}

				e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;

				foreach(MGUIControl c in m_designer.m_editorDialog.Controls)
				{
					c.UpdateRects();
					c.Draw(e.Graphics);
				}

				IDesignerHost host = DesignerHost;
				ISelectionService ss = SelectionService;
				if (m_designer.m_currentSelection != null && host != null && ss != null) 
				{
					foreach(object comp in m_designer.m_currentSelection)
					{
						if (comp is IComponent)
						{
							ControlDesigner des = host.GetDesigner((IComponent)comp) as ControlDesigner;
							if (des != null)
							{
								bool primary = comp == ss.PrimarySelection;
								des.DrawAdornments(e.Graphics, primary);
							}
						}
					}
				}
			}
			else
			{
				StringFormat format = new StringFormat();
				format.Alignment = StringAlignment.Center;
				format.LineAlignment = StringAlignment.Center;
				g.DrawString("Drag Controls onto this dialog surface from the toolbox.", Font, SystemBrushes.ControlDark, dialogPanel.Bounds, format);
			}
		}

		private void captionPanel_Paint(object sender, PaintEventArgs e)
		{
			try
			{
				if (GUIProperties.controlsPicture == null)
					return;
				Graphics g = e.Graphics;
				g.SmoothingMode = SmoothingMode.AntiAlias;
				g.DrawImage(GUIProperties.controlsPicture, new Rectangle(0,0,captionPanel.Width,captionPanel.Height),
					GUIProperties.captionRectangle, GraphicsUnit.Pixel);
				g.DrawString(m_dialog.Text, GUIProperties.font, Brushes.Black, 3, 3);
				g.DrawString(m_dialog.Text, GUIProperties.font, Brushes.White, 2, 2);
			}
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message + "\n" + ex.StackTrace);
            }
        }


		private void dialogPanel_DragEnter(object sender, System.Windows.Forms.DragEventArgs e)
		{
			m_dragObject = e.Data.GetData(typeof(DragObject)) as DragObject;
			if (m_dragObject != null)
			{
				if (m_dragObject.DragControl != dialogPanel)
				{
					m_dragObject = null;
					e.Effect = DragDropEffects.None;
				}
				else
				{
					e.Effect = DragDropEffects.Move;
				}
			}
			else
			{
				IToolboxService ts = ToolboxService;
				if (ts != null && ts.IsToolboxItem(e.Data, DesignerHost))
				{
					e.Effect = DragDropEffects.Copy;
				}
			}
		}

		private void dialogPanel_DragDrop(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (m_dragObject != null)
			{
				DragObject d = m_dragObject;
				m_dragObject = null;
				d.Drop(e.X, e.Y);
			}
			else 
			{
				IToolboxService ts = ToolboxService;
				if (ts != null && ts.IsToolboxItem(e.Data, DesignerHost))
				{
					ToolboxItem item = ts.DeserializeToolboxItem(e.Data, DesignerHost);
					Point pt = new Point(e.X, e.Y);
					pt = PointToClient(pt);
					InvokeToolboxItem(item, pt.X, pt.Y);
				}
			}
		}

		private void dialogPanel_DragLeave(object sender, System.EventArgs e)
		{
			if (m_dragObject != null)
			{
				m_dragObject.ClearFeedback();
				m_dragObject = null;
			}
		}

		private void dialogPanel_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left)
			{
				ISelectionService ss = SelectionService;
				if (ss != null)
				{
					object hitObject = null;

					if (m_hitControl == null)
					{
						IDesignerHost host = DesignerHost;
						if (host != null)
						{
							hitObject = host.RootComponent;
						}
					}
					else
					{
						hitObject = m_hitControl;
					}

					if (hitObject != null)
					{
                        
						ss.SetSelectedComponents(new object[] {hitObject}, SelectionTypes.Click);
					}
				}
			}
		}

		private void dialogPanel_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			object newHitObject = null;
			MGUIControl newHitShape = null;

			IDesignerHost host = DesignerHost;
			if (host != null)
			{
				for (int i=m_designer.m_editorDialog.Controls.Count-1;i>=0;i--)
				{
					MGUIControl s =  m_designer.m_editorDialog.Controls[i] as MGUIControl;
					ControlDesigner des = host.GetDesigner(s) as ControlDesigner;
					if (des != null)
					{
						newHitObject = des.GetHitTest(e.X, e.Y);
						if (newHitObject != null)
						{
							newHitShape = s;
							Cursor = des.GetCursor(newHitObject);
							break;
						}
					}
				}
			}

			m_hitObject = newHitObject;
			m_hitControl = newHitShape;

			if (newHitObject == null)
			{
				IToolboxService ts = ToolboxService;
				if (ts == null || !ts.SetCursor())
				{
					Cursor = Cursors.Default;
				}
				else
				{
					m_toolboxItem = ts.GetSelectedToolboxItem(DesignerHost);
				}
			}

			if (e.Button == MouseButtons.Left && m_hitObject != null)
			{
				//	Rectangle bbox = dialogPanel.Bounds;
				//	if (m_hitControl != null)
				//		bbox=m_hitControl.BoundingBox;
				DoDragDrop(new DragObject(m_designer, dialogPanel, m_hitObject, e.X, e.Y), DragDropEffects.Move);
				//if (m_hitControl != null)
				//	m_hitControl.Draw(dialogPanel .CreateGraphics());
				dialogPanel.Invalidate();
			}
		}

		private void dialogPanel_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left && m_toolboxItem != null)
			{
				InvokeToolboxItem(m_toolboxItem, e.X, e.Y);
				m_toolboxItem = null;
				Cursor = Cursors.Default;
			}
           
        }

		private void dialogPanel_GiveFeedback(object sender, System.Windows.Forms.GiveFeedbackEventArgs e)
		{
			if (m_dragObject != null)
			{
				m_dragObject.GiveFeedback();
			}
		}

		private void DesignerView_Load(object sender, System.EventArgs e)
		{
			dialogPanel.Invalidate();
		}

		private void dialogPanel_DragOver(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (m_dragObject != null)
			{
				m_dragObject.GiveFeedback();
			}
		}
	}

	internal class DialogEditor : Panel
	{
		protected override void OnGiveFeedback(GiveFeedbackEventArgs gfbevent)
		{
			base.OnGiveFeedback (gfbevent);
		}

	}
}
