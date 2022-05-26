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
	internal class DragObject
	{
		private DialogDesigner m_designer;
		private Control m_dragControl;
		private ArrayList m_dragControls;
		private object m_hitObject;
		private int m_initialX;
		private int m_initialY;
		private Point m_screenOffset;
		private Point m_mouseOffset;
		private bool m_removeOldDrag;

		internal DragObject(DialogDesigner designer, Control dragControl, object hitTestObject, int initialX, int initialY)
		{
			m_designer = designer;
			m_dragControl = dragControl;
			m_hitObject = hitTestObject;
			m_initialX = initialX;
			m_initialY = initialY;
			m_mouseOffset = new Point(0, 0);
			m_screenOffset = new Point(0, 0);
			m_screenOffset = dragControl.PointToScreen(m_screenOffset);

			ISelectionService ss = designer.GetServiceP(typeof(ISelectionService)) as ISelectionService;
			IDesignerHost host = designer.GetServiceP(typeof(IDesignerHost)) as IDesignerHost;

			m_dragControls = new ArrayList();

			if (ss != null && host != null)
			{
				ICollection selectedObjects = ss.GetSelectedComponents();
				foreach(object o in selectedObjects)
				{
					IComponent comp = o as IComponent;
					if (comp != null)
					{
						ControlDesigner sd = host.GetDesigner(comp) as ControlDesigner;
						if (sd != null)
						{
							m_dragControls.Add(sd);
						}
					}
				}
			}
		}

		public Control DragControl
		{
			get
			{
				return m_dragControl;
			}
		}

		public void ClearFeedback()
		{
			if (m_removeOldDrag)
			{
				m_removeOldDrag = false;
				foreach(ControlDesigner sd in m_dragControls)
				{
					sd.DrawDragFeedback(m_hitObject, m_dragControl.BackColor, m_screenOffset, m_mouseOffset,m_designer.m_editorDialog);
				}
			}
		}

		public void Drop(int x, int y)
		{
			ClearFeedback();
			Point pt = new Point(x, y);
			pt = m_dragControl.PointToClient(pt);
			x = pt.X - m_initialX;
			y = pt.Y - m_initialY;

            IDesignerHost host = (IDesignerHost)m_designer.GetServiceP(typeof(IDesignerHost));
            IComponentChangeService cs = (IComponentChangeService)m_designer.GetServiceP(typeof(IComponentChangeService));
            if (m_dragControls.Count > 1)
			{
				
				if (host != null)
				{
					using(DesignerTransaction trans = host.CreateTransaction("Drag " + m_dragControls.Count + " components"))
					{
                        
						foreach(ControlDesigner sd in m_dragControls)
						{
                            cs.OnComponentChanging(sd.Component, null);
                            sd.Drag(m_hitObject, x, y,m_designer.m_editorDialog);
                            cs.OnComponentChanged(sd.Component, null, null, null);
                        }
						trans.Commit();
					}
				}
			}
			else
			{
                    foreach (ControlDesigner sd in m_dragControls)
                    {
                        cs.OnComponentChanging(sd.Component,null);
                        sd.Drag(m_hitObject, x, y, m_designer.m_editorDialog);
                        cs.OnComponentChanged(sd.Component,null, null, null);
                    }
            }
		}


		public void GiveFeedback()
		{
			ClearFeedback();

			Point currentPosition = Control.MousePosition;
			m_mouseOffset.X = currentPosition.X - m_screenOffset.X - m_initialX;
			m_mouseOffset.Y = currentPosition.Y - m_screenOffset.Y - m_initialY;

			foreach(ControlDesigner sd in m_dragControls)
			{
				sd.DrawDragFeedback(m_hitObject, m_dragControl.BackColor, m_screenOffset, m_mouseOffset,m_designer.m_editorDialog);
			}

			m_removeOldDrag = true;
		}
	}
}
