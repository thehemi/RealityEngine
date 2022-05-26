#region Using directives

using System;
using System.ComponentModel;
using System.Collections;
using System.Diagnostics;
using System.Text;
using System.Drawing;

#endregion

namespace ScriptingSystem
{
    [Serializable]
   [ToolboxItemFilter("ArtificialStudios.GuiControls", ToolboxItemFilterType.Allow)]
    public abstract class MGUIControl : Component
   {
	   private Point location = new Point(1,1);
	   public  virtual Point Location
	   {
		   get  {  return location; }
		   set   { location = value; }
	   }

	   private Size size = new Size(60,20);
	
	   public  virtual Size Size
	   {
		   get{  return size; }
		   set  { size = value;}
	   }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Rectangle BoundingBox
        {
			get
			{
				return new Rectangle(location,size);
			}
			set
			{
				location = value.Location;
				size = value.Size;
			}
		}
	
		internal virtual void  UpdateRects()
		{
		}
        internal abstract void Draw(Graphics g);
    }
}
