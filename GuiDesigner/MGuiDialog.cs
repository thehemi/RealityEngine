#region Using directives

using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

#endregion

namespace ScriptingSystem
{
    [Serializable]
    [Designer(typeof(ScriptingSystem.DialogDesigner), typeof(System.ComponentModel.Design.IRootDesigner))]
    [ToolboxItemFilter("ArtificialStudios.GuiControls", ToolboxItemFilterType.Prevent)]
    public class MGUIDialog : Component
    {
        public MGUIDialog()
        {

        }

        public ArrayList m_controls = new ArrayList();
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
		[Browsable(false)]
		public ArrayList Controls
		{
			get
			{
				return m_controls;
			}
			set
			{
				m_controls = value;
			}
		}

        private string text = "Dialog";
        [DefaultValue("Dialog")]
        public string Text
        {
            get { return text; }
            set { text  = value; }
        }

		private bool enableCaption = false;
        [DefaultValue(false)]
        public bool EnableCaption
		{
			get { return enableCaption; }
			set { enableCaption  = value; }
		}

		private bool enableMouse = false;
        [DefaultValue(false)]
        public bool EnableMouseInput
		{
			get { return enableMouse; }
			set { enableMouse  = value; }
		}

		private bool drawGrid = true;
        [DefaultValue(true)]
        [DesignOnly(true)]
        public bool DrawGrid
		{
			get { return drawGrid; }
			set { drawGrid  = value; }
		}

		private Size gridSize = new Size(10,10);
        [DesignOnly(true)]
        public Size GridSize
		{
			get { return gridSize; }
			set { gridSize  = value; }
		}

		private Point location = new Point(1,1);
		public  virtual Point Location
		{
			get  {  return location; }
			set   { location = value; }
		}

		private Size size = new Size(640,480);
		public  virtual Size Size
		{
			get{  return size; }
			set  { size = value;}
		}

        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        [Browsable(false)]
        public  virtual Rectangle BoundingBox
		{
			get{ return new Rectangle(location,size); }
			set
			{
				location = value.Location;
				size = value.Size;
			}
		}
    }
}
