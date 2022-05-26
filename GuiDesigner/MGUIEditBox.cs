using System.Drawing;
using System;
using System.Collections;
using System.Text;
using System.ComponentModel;

namespace ScriptingSystem
{
	/// <summary>
	/// Summary description for MGUIEditBox.
	/// </summary>
	[Designer(typeof(ScriptingSystem.DefaultControlDesigner))]
	public class MGUIEditBox : MGUIControl
	{
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

        private int m_borderWidth = 5;
        [DefaultValue(5)]
        public int BorderWidth
        {
            get { return m_borderWidth; }
            set { m_borderWidth = value; }
        }


        internal override void Draw(System.Drawing.Graphics g)
		{
			try
			{
				g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
				g.CompositingMode = System.Drawing.Drawing2D.CompositingMode.SourceOver;

				Rectangle m_rcText = BoundingBox;
                m_rcText.Inflate(-m_borderWidth, -m_borderWidth);

                // Update the render rectangles
				Rectangle[] m_rcRender = new Rectangle[9];
				m_rcRender[0] = m_rcText;
				m_rcRender[1]=  Rectangle.FromLTRB( BoundingBox.Left, BoundingBox.Top, m_rcText.Left, m_rcText.Top );
				m_rcRender[2]=  Rectangle.FromLTRB(  m_rcText.Left, BoundingBox.Top, m_rcText.Right, m_rcText.Top );
				m_rcRender[3]=  Rectangle.FromLTRB( m_rcText.Right, BoundingBox.Top, BoundingBox.Right, m_rcText.Top );
				m_rcRender[4]=  Rectangle.FromLTRB(  BoundingBox.Left, m_rcText.Top, m_rcText.Left, m_rcText.Bottom );
				m_rcRender[5]=  Rectangle.FromLTRB(  m_rcText.Right, m_rcText.Top, BoundingBox.Right, m_rcText.Bottom );
				m_rcRender[6]=  Rectangle.FromLTRB(  BoundingBox.Left, m_rcText.Bottom, m_rcText.Left, BoundingBox.Bottom );
				m_rcRender[7]=  Rectangle.FromLTRB(  m_rcText.Left, m_rcText.Bottom, m_rcText.Right, BoundingBox.Bottom );
				m_rcRender[8]=  Rectangle.FromLTRB(  m_rcText.Right, m_rcText.Bottom, BoundingBox.Right, BoundingBox.Bottom );

				for (int i=0;i<9;i++)
					g.DrawImage(GUIProperties.controlsPicture, m_rcRender[i],
						GUIProperties.editBoxRectangles[i], GraphicsUnit.Pixel);

                m_rcText.Offset(4, 4);

                StringFormat format = new StringFormat();
                format.Alignment = StringAlignment.Near;
                format.LineAlignment = StringAlignment.Near;
                g.DrawString(Text, GUIProperties.font, Brushes.Black, m_rcText, format);
            }
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message + "\n" + ex.StackTrace);
            }
        }
	}
}
