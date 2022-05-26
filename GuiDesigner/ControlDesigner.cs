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
    public abstract class ControlDesigner : ComponentDesigner
    {
        private static readonly Size m_adornmentDimensions = new Size(7, 7);

        protected static readonly object HitMove = new object();

        public static Size AdornmentDimensions
        {
            get
            {
                return m_adornmentDimensions;
            }
        }

        [Browsable(false)]
        [DesignOnly(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        protected abstract Rectangle BoundingBox { get; set; }

        public abstract object GetHitTest(int x, int y);

        public abstract Cursor GetCursor(object hitObject);

        public abstract void Drag(object hitObject, int x, int y,MGUIDialog editorDialog);

        public abstract void DrawAdornments(Graphics g, bool primary);

        public abstract void DrawDragFeedback(object hitObject, Color backColor, Point screenOffset, Point mouseOffset,MGUIDialog editorDialog);
    }

    public class DefaultControlDesigner : ControlDesigner
    {
		private static readonly object HitBottom = new object();
		private static readonly object HitLeft = new object();
		private static readonly object HitRight = new object();
		private static readonly object HitTop = new object();
		private static readonly object HitLowerLeft = new object();
		private static readonly object HitLowerRight = new object();
		private static readonly object HitUpperLeft = new object();
		private static readonly object HitUpperRight = new object();


		protected override Rectangle BoundingBox 
		{ 
			get
			{
				return Control.BoundingBox;
			}
			set
			{
				Control.BoundingBox = value;
			}
		}

		protected virtual MGUIControl Control
		{
			get
			{
				return (MGUIControl)Component;
			}
		}

		private Rectangle AdjustBounds(object hitObject, Rectangle bounds, int x, int y, out bool adjusted)
		{
			
			adjusted = false;

			bounds.Width = Math.Max(bounds.Width, 1);
			bounds.Height = Math.Max(bounds.Height, 1);	

			if (hitObject == HitLeft || hitObject == HitUpperLeft || hitObject == HitLowerLeft || hitObject == HitMove)
			{
					bounds.X += x;
				adjusted = true;
			}

			if (hitObject == HitTop || hitObject == HitUpperLeft || hitObject == HitUpperRight || hitObject == HitMove)
			{
					bounds.Y += y;
				adjusted = true;
			}

			if (hitObject == HitLeft || hitObject == HitUpperLeft || hitObject == HitLowerLeft)
			{
					bounds.Width -= x;
				adjusted = true;
			}

			if (hitObject == HitRight || hitObject == HitUpperRight || hitObject == HitLowerRight)
			{
					bounds.Width += x;
				adjusted = true;
			}

			if (hitObject == HitTop || hitObject == HitUpperLeft || hitObject == HitUpperRight)
			{
					bounds.Height -= y;
				adjusted = true;
			}

			if (hitObject == HitBottom || hitObject == HitLowerLeft || hitObject == HitLowerRight)
			{
					bounds.Height += y;
				adjusted = true;
			}

			bounds.Width = Math.Max(bounds.Width, 1);
			bounds.Height = Math.Max(bounds.Height, 1);
			

			return bounds;
		}

		public override object GetHitTest(int x, int y)
		{
			Rectangle bounds = BoundingBox;
			bounds.Inflate(AdornmentDimensions);

			if (!bounds.Contains(x, y))
			{
				return null;
			}

			bounds = BoundingBox;

			int adornmentOffset = AdornmentDimensions.Width / 2;
			Rectangle adornmentRect = new Rectangle(bounds.X - adornmentOffset, bounds.Y - adornmentOffset, AdornmentDimensions.Width, AdornmentDimensions.Height);

			if (adornmentRect.Contains(x, y))
			{
				return HitUpperLeft;
			}

			adornmentRect.Y = bounds.Bottom - adornmentOffset;
			if (adornmentRect.Contains(x, y))
			{
				return HitLowerLeft;
			}

			adornmentRect.X = bounds.Right - adornmentOffset;
			if (adornmentRect.Contains(x, y))
			{
				return HitLowerRight;
			}

			adornmentRect.Y = bounds.Top - adornmentOffset;
			if (adornmentRect.Contains(x, y))
			{
				return HitUpperRight;
			}

			adornmentRect.X = bounds.Left + bounds.Width / 2 - adornmentOffset;
			if (adornmentRect.Contains(x, y))
			{
				return HitTop;
			}

			adornmentRect.Y = bounds.Bottom - adornmentOffset;
			if (adornmentRect.Contains(x, y))
			{
				return HitBottom;
			}

			adornmentRect.X = bounds.Left - adornmentOffset;
			adornmentRect.Y = bounds.Top + bounds.Height / 2 - adornmentOffset;
			if (adornmentRect.Contains(x, y))
			{
				return HitLeft;
			}

			adornmentRect.X = bounds.Right - adornmentOffset;
			if (adornmentRect.Contains(x, y))
			{
				return HitRight;
			}

			return HitMove;
		}

		public override Cursor GetCursor(object hitObject)
		{
			if (hitObject == HitLeft || hitObject == HitRight)
			{
				return Cursors.SizeWE;
			}

			if (hitObject == HitTop || hitObject == HitBottom)
			{
				return Cursors.SizeNS;
			}

			if (hitObject == HitUpperLeft || hitObject == HitLowerRight)
			{
				return Cursors.SizeNWSE;
			}

			if (hitObject == HitUpperRight || hitObject == HitLowerLeft)
			{
				return Cursors.SizeNESW;
			}

			return Cursors.SizeAll;
		}

		public override void Drag(object hitObject, int x, int y,MGUIDialog editorDialog)
		{
			IDesignerHost host = (IDesignerHost)GetService(typeof(IDesignerHost));

			if (host != null)
			{
				
				Rectangle bounds = Control.BoundingBox;

				bool adjusted;
				bounds = AdjustBounds(hitObject, bounds, x, y, out adjusted);

				//Snap to grid
				int hGSX = editorDialog.GridSize.Width/2;
				int hGSY  = editorDialog.GridSize.Width/2;
				int xmod = bounds.X % editorDialog.GridSize.Width;
				if (xmod < hGSX) bounds.X -= xmod;
				else bounds.X += editorDialog.GridSize.Width-xmod;

				int ymod = bounds.Y % editorDialog.GridSize.Height;
				if (ymod < hGSY) bounds.Y -= ymod;
				else bounds.Y += editorDialog.GridSize.Height-ymod;

				xmod = bounds.Width % editorDialog.GridSize.Width;
				if (xmod < hGSX) bounds.Width -= xmod;
				else  bounds.Width += editorDialog.GridSize.Width-xmod;

				ymod = bounds.Height % editorDialog.GridSize.Height;
				if (ymod < hGSY) bounds.Height -= ymod;
				else bounds.Height += editorDialog.GridSize.Height-ymod;


				if (adjusted)
				{
					DesignerTransaction trans = null;

					if (hitObject == HitMove)
					{
						trans = host.CreateTransaction("Move " + Component.Site.Name);
					}
					else
					{
						trans = host.CreateTransaction("Resize " + Component.Site.Name);
					}

					using(trans)
					{
						Control.BoundingBox = bounds;
						trans.Commit();
					}
				}
			}
		}


		public override void DrawAdornments(Graphics g, bool primary)
		{
			
			int adornmentOffset = AdornmentDimensions.Width / 2;
			Rectangle bounds = BoundingBox;
			Rectangle adornmentRect = new Rectangle(bounds.X - adornmentOffset, bounds.Y - adornmentOffset, AdornmentDimensions.Width, AdornmentDimensions.Height);

			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);

			adornmentRect.X = bounds.X + bounds.Width / 2 - adornmentOffset;
			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);

			adornmentRect.X = bounds.Right - adornmentOffset;
			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);

			adornmentRect.Y = bounds.Y + bounds.Height / 2 - adornmentOffset;
			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);

			adornmentRect.Y = bounds.Bottom - adornmentOffset;
			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);

			adornmentRect.X = bounds.X + bounds.Width / 2 - adornmentOffset;
			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);

			adornmentRect.X = bounds.X - adornmentOffset;
			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);

			adornmentRect.Y = bounds.Y + bounds.Height / 2 - adornmentOffset;
			ControlPaint.DrawGrabHandle(g, adornmentRect, primary, true);
		}

		public override void DrawDragFeedback(object hitObject, Color backColor, Point screenOffset, Point mouseOffset,MGUIDialog editorDialog)
		{
			bool adjusted;
			Rectangle bounds = AdjustBounds(hitObject, BoundingBox, mouseOffset.X, mouseOffset.Y, out adjusted);
			
			//Snap to grid
			int hGSX = editorDialog.GridSize.Width/2;
			int hGSY  = editorDialog.GridSize.Width/2;
			int xmod = bounds.X % editorDialog.GridSize.Width;
			if (xmod < hGSX) bounds.X -= xmod;
			else bounds.X += editorDialog.GridSize.Width-xmod;

			int ymod = bounds.Y % editorDialog.GridSize.Height;
			if (ymod < hGSY) bounds.Y -= ymod;
			else bounds.Y += editorDialog.GridSize.Height-ymod;

			xmod = bounds.Width % editorDialog.GridSize.Width;
			if (xmod < hGSX) bounds.Width -= xmod;
			else  bounds.Width += editorDialog.GridSize.Width-xmod;

			ymod = bounds.Height % editorDialog.GridSize.Height;
			if (ymod < hGSY) bounds.Height -= ymod;
			else bounds.Height += editorDialog.GridSize.Height-ymod;

			bounds.Offset(screenOffset);
			if (adjusted)
			{
				
				Point startPoint = new Point(bounds.X, bounds.Y);
				Point endPoint = new Point(bounds.Right, bounds.Y);

				ControlPaint.DrawReversibleLine(startPoint, endPoint, backColor);

				startPoint = endPoint;
				endPoint.Y = bounds.Bottom;
				ControlPaint.DrawReversibleLine(startPoint, endPoint, backColor);

				startPoint = endPoint;
				endPoint.X = bounds.X;
				ControlPaint.DrawReversibleLine(startPoint, endPoint, backColor);

				startPoint = endPoint;
				endPoint.Y = bounds.Y;
				ControlPaint.DrawReversibleLine(startPoint, endPoint, backColor);
			}
		}
    }
}
