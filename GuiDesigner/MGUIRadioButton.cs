#region Using directives

using System.Drawing;
using System;
using System.Collections;
using System.Text;
using System.ComponentModel;

#endregion

namespace ScriptingSystem
{
    [Designer(typeof(ScriptingSystem.DefaultControlDesigner))]
    [ToolboxItemFilter("ArtificialStudios.GuiControls", ToolboxItemFilterType.Allow)]
    public class MGUIRadioButton : MGUICheckBox
    {
        public MGUIRadioButton()
        {

        }

		internal override void UpdateRects()
		{
			m_rcButton = this.BoundingBox;
			m_rcButton.Width = m_rcButton.Height;

			m_rcText = this.BoundingBox;
			m_rcText.X += (int) ( 1.25f *  m_rcButton.Width );
			m_rcText.Width -= (int) ( 1.25f *  m_rcButton.Width );
		}

        internal override void Draw(System.Drawing.Graphics g)
        {
			try
			{
				g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
				g.CompositingMode = System.Drawing.Drawing2D.CompositingMode.SourceOver;
				
				
				g.DrawImage(GUIProperties.controlsPicture, m_rcButton,
					GUIProperties.radioButtonBackRectangle, GraphicsUnit.Pixel);
				
				if (m_checked)
				{
					g.DrawImage(GUIProperties.controlsPicture, m_rcButton,
						GUIProperties.radioButtonRectangle, GraphicsUnit.Pixel);
				}
					
				StringFormat format = new StringFormat();
				format.Alignment = StringAlignment.Near;
				format.LineAlignment = StringAlignment.Center;
                format.FormatFlags = StringFormatFlags.FitBlackBox | StringFormatFlags.NoWrap;

                g.DrawString(Text, GUIProperties.font, Brushes.White, m_rcText,format);
			}
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message + "\n" + ex.StackTrace);
            }
        }
    }
}
