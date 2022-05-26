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
	public class MGUIScrollBar : MGUIControl
	{
		internal Rectangle m_rcUpButton,m_rcDownButton,m_rcTrack;
		public MGUIScrollBar()
		{

		}

		int m_pageSize = 1;
        [DefaultValue(1)]
        public int PageSize
		{
			get
			{
				return m_pageSize;
			}
			set
			{
				m_pageSize=value;
			}
		}

		int m_trackPos;
        [DefaultValue(0)]
        public int TrackPos
		{
			get
			{
				return m_trackPos;
			}
			set
			{
				m_trackPos=value;
			}
		}

		int m_trackStart;
        [DefaultValue(0)]
        public int TrackStart
		{
			get
			{
				return m_trackStart;
			}
			set
			{
				m_trackStart=value;
			}
		}

		int m_trackEnd = 1;
        [DefaultValue(1)]
		public int TrackEnd
		{
			get
			{
				return m_trackEnd;
			}
			set
			{
				m_trackEnd=value;
			}
		}

		internal override void UpdateRects()
		{
			m_rcUpButton =  Rectangle.FromLTRB ( BoundingBox.Left, BoundingBox.Top,
				BoundingBox.Right, BoundingBox.Top + BoundingBox.Width );
			m_rcDownButton =Rectangle.FromLTRB( BoundingBox.Left, BoundingBox.Bottom - BoundingBox.Width,
				BoundingBox.Right, BoundingBox.Bottom );
			m_rcTrack = Rectangle.FromLTRB( m_rcUpButton.Left, m_rcUpButton.Bottom,
				m_rcDownButton.Right, m_rcDownButton.Top );
		}


        internal override void Draw(System.Drawing.Graphics g)
        {
			try
			{
				g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
				g.CompositingMode = System.Drawing.Drawing2D.CompositingMode.SourceOver;
				
				g.DrawImage(GUIProperties.controlsPicture, BoundingBox,
                    GUIProperties.scrollBarTrackRectangle, GraphicsUnit.Pixel);

                g.DrawImage(GUIProperties.controlsPicture, m_rcUpButton,
					GUIProperties.scrollBarUpArrowRectangle, GraphicsUnit.Pixel);

				g.DrawImage(GUIProperties.controlsPicture, m_rcDownButton,
					GUIProperties.scrollBarDownArrowRectangle, GraphicsUnit.Pixel);
			}
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message + "\n" + ex.StackTrace);
            }
        }
	}
}
