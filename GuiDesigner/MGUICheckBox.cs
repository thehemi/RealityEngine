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
    public class MGUICheckBox : MGUIButton
    {
		internal Rectangle m_rcButton,m_rcText;
        public MGUICheckBox()
        {

        }

		internal bool m_checked = false;
        [DefaultValue(false)]
        public bool Checked
		{
			get
			{
				return m_checked;
			}
			set
			{
				m_checked = value;
			}
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
					GUIProperties.checkBoxBackRectangle, GraphicsUnit.Pixel);
				
				if (m_checked)
				{
					g.DrawImage(GUIProperties.controlsPicture, m_rcButton,
						GUIProperties.checkBoxRectangle, GraphicsUnit.Pixel);
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
