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
    [ToolboxItemFilter("ArtificialStudios.GuiControls", ToolboxItemFilterType.Require)]
    internal class DialogDesigner : ComponentDesigner, IRootDesigner, IToolboxUser
    {
        private DesignerView m_view;
        public ICollection m_currentSelection;
        private IMenuCommandService m_menuCommandService;
        private MenuCommand[] m_menuCommands;

        public MGUIDialog m_editorDialog;


        public DialogDesigner()
        {
            
        }

		public  object GetServiceP(Type serviceType)
		{
			return base.GetService (serviceType);
		}


        #region IRootDesigner Members

        public object GetView(ViewTechnology technology)
        {
            try
            {
                if (technology != ViewTechnology.WindowsForms)
                    throw new Exception("Technology not supported");
                if (m_view == null)
                    m_view = new DesignerView(m_editorDialog,this);
                return m_view;
            }
            catch
            {
                MessageBox.Show("Error");
                return null;
            }
        }

        public ViewTechnology[] SupportedTechnologies
        {
            get { return new ViewTechnology[] { ViewTechnology.WindowsForms }; }
        }


        public override void Initialize(IComponent component)
        {
			base.Initialize(component);
            MGUIDialog dialog = component as MGUIDialog;
            if (dialog != null)
                m_editorDialog = dialog;

            IComponentChangeService cs = (IComponentChangeService)GetService(typeof(IComponentChangeService));
            if (cs != null)
            {
                cs.ComponentChanged += new ComponentChangedEventHandler(OnComponentChanged);
                cs.ComponentChanging += new ComponentChangingEventHandler(OnComponentChanging);
            }

            ISelectionService ss = (ISelectionService)GetService(typeof(ISelectionService));
            if (ss != null)
            {
                ss.SetSelectedComponents(new object[] { component }, SelectionTypes.Replace);
                ss.SelectionChanged += new EventHandler(OnSelectionChanged);
            }      
        }



        #endregion

        #region IToolboxUser Members

        bool IToolboxUser.GetToolSupported(ToolboxItem tool)
        {
            
            return true;
        }

        void IToolboxUser.ToolPicked(ToolboxItem tool)
        {
			try
			{
				m_view.InvokeToolboxItem(tool, m_view.dialogPanel.Width / 2, m_view.dialogPanel.Height / 2);
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message + ex.StackTrace);
			}
			//MessageBox.Show("picked");
        }

#endregion

		private void InvalidateShapeBounds(object shape)
		{
			PropertyDescriptor prop = TypeDescriptor.GetProperties(shape)["BoundingBox"];
			if (m_view != null && prop != null && prop.PropertyType == typeof(Rectangle))
			{
				Rectangle bounds = (Rectangle)prop.GetValue(shape);
				bounds.Inflate(ControlDesigner.AdornmentDimensions);
				m_view.dialogPanel.Invalidate(bounds);
			}
		}
        private void OnSelectionChanged(object sender, EventArgs e)
        {
            if (m_currentSelection != null && m_view != null)
            {
                foreach (object o in m_currentSelection)
                {
                    InvalidateShapeBounds(o);
                }
            }

            m_currentSelection = ((ISelectionService)sender).GetSelectedComponents();

            if (m_currentSelection != null && m_view != null)
            {
                foreach (object o in m_currentSelection)
                {
                    InvalidateShapeBounds(o);
                }
            }

            if (m_menuCommandService != null)
            {
                bool enableEditCommands = false;

                if (m_currentSelection != null && m_currentSelection.Count > 0)
                {
                    enableEditCommands = !((ISelectionService)sender).GetComponentSelected(Component);
                }

                MenuCommand mc = m_menuCommandService.FindCommand(StandardCommands.Cut);
                if (mc != null) mc.Enabled = enableEditCommands;

                mc = m_menuCommandService.FindCommand(StandardCommands.Copy);
                if (mc != null) mc.Enabled = enableEditCommands;

                mc = m_menuCommandService.FindCommand(StandardCommands.Delete);
                if (mc != null) mc.Enabled = enableEditCommands;
            }
        }

      
        private void OnComponentChanged(object sender, ComponentChangedEventArgs ce)
        {
            if (m_view != null && ce.Component is MGUIControl)
            {
                InvalidateShapeBounds(ce.Component);
            }
            else if (ce.Component is MGUIDialog)
            {
                m_view.dialogPanel.Invalidate();
                m_view.captionPanel.Invalidate();
            }
        }

        private void OnComponentChanging(object sender, ComponentChangingEventArgs ce)
        {
            if (m_view != null && ce.Component is MGUIControl)
            {
                InvalidateShapeBounds(ce.Component);
            }
            else if (ce.Component is MGUIDialog)
            {
                m_view.dialogPanel.Invalidate();
                m_view.captionPanel.Invalidate();
            }
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (m_view != null)
                {
                    m_view.Dispose();
                }

                IComponentChangeService cs = (IComponentChangeService)GetService(typeof(IComponentChangeService));
                if (cs != null)
                {
                    cs.ComponentChanged -= new ComponentChangedEventHandler(OnComponentChanged);
                    cs.ComponentChanging -= new ComponentChangingEventHandler(OnComponentChanging);
                }

                ISelectionService ss = (ISelectionService)GetService(typeof(ISelectionService));
                if (ss != null)
                {
                    ss.SelectionChanged -= new EventHandler(OnSelectionChanged);
                }

                if (m_menuCommands != null && m_menuCommandService != null)
                {
                    foreach (MenuCommand mc in m_menuCommands)
                    {
                        m_menuCommandService.RemoveCommand(mc);
                    }
                    m_menuCommands = null;
                }
            }

            base.Dispose(disposing);
        }
    }
}
