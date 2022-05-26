#region Using directives

using System.Drawing;
using System;
using System.Collections;
using System.Text;
using System.ComponentModel;

#endregion

namespace ScriptingSystem
{
    [Serializable]
    [Designer(typeof(ScriptingSystem.DefaultControlDesigner))]
    [ToolboxItemFilter("ArtificialStudios.GuiControls", ToolboxItemFilterType.Allow)]
    public class MGUIButton : MGUIControl
    {
        public MGUIButton()
        {

        }

		private string text = "";
        [DefaultValue("")]
        public string Text
		{
			get
			{
				return text;
			}
			set
			{
				text = value;
			}
		}

        internal override void Draw(System.Drawing.Graphics g)
        {
			try
			{
				g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
				g.CompositingMode = System.Drawing.Drawing2D.CompositingMode.SourceOver;
				g.DrawImage(GUIProperties.controlsPicture, BoundingBox,
					GUIProperties.buttonRectangle, GraphicsUnit.Pixel);
					
				StringFormat format = new StringFormat();
				format.Alignment = StringAlignment.Center;
				format.LineAlignment = StringAlignment.Center;
                format.FormatFlags = StringFormatFlags.FitBlackBox | StringFormatFlags.NoWrap;

                g.DrawString(Text, GUIProperties.font, Brushes.White, BoundingBox,format);
			}
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message + "\n" + ex.StackTrace);
            }
        }
    }
}
