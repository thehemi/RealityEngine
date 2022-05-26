#region Using directives

using System;
using System.Collections;
using System.Text;
using System.Drawing;
using System.Windows.Forms;

#endregion

namespace ScriptingSystem
{
    internal class GUIProperties
    {
        public static Bitmap controlsPicture;

        public static Color backColorTopRight = Color.FromArgb(32, 32, 55);
        public static Color backColorTopLeft = Color.FromArgb(32, 32, 55);
        public static Color backColorBottomRight = Color.FromArgb(32, 32, 55);
        public static Color backColorBottomLeft = Color.FromArgb(32, 32, 55);

        public static Font font = new Font("Arial", 8);

        public static Rectangle captionRectangle = new Rectangle(17, 269, 241 - 17, 287 - 269);

        public static Rectangle buttonRectangle = new Rectangle(0, 0, 136, 54);

        public static Rectangle checkBoxRectangle = new Rectangle(27, 54, 54 - 27, 81 - 54);
        public static Rectangle checkBoxBackRectangle = new Rectangle(0, 54, 27, 81 - 54);

        public static Rectangle radioButtonRectangle = new Rectangle(81, 54, 108 - 81, 81 - 54);
        public static Rectangle radioButtonBackRectangle = new Rectangle(54, 54, 81 - 54, 81 - 54);

        public static Color buttonTextColor = Color.White;
        public static Color checkBoxTextColor = Color.White;

        public static Rectangle scrollBarTrackRectangle = new Rectangle(243, 144, 265 - 243, 155 - 144);
        public static Rectangle scrollBarUpArrowRectangle = new Rectangle(243, 124, 265 - 243, 144 - 124);
        public static Rectangle scrollBarDownArrowRectangle = new Rectangle(243, 155, 265 - 243, 176 - 155);
        public static Rectangle scrollBarButtonRectangle = new Rectangle(266, 123, 286 - 266, 167 - 123);


        // Element assignment:
        //   0 - text area
        //   1 - top left border
        //   2 - top border
        //   3 - top right border
        //   4 - left border
        //   5 - right border
        //   6 - lower left border
        //   7 - lower border
        //   8 - lower right border
        public static Rectangle[] editBoxRectangles = new Rectangle[9];

        static GUIProperties()
        {
            try
            {
                editBoxRectangles[0] = new Rectangle(14, 90, 241 - 14, 113 - 90);
                editBoxRectangles[1] = new Rectangle(8, 82, 14 - 8, 90 - 82);
                editBoxRectangles[2] = new Rectangle(14, 82, 241 - 14, 90 - 82);
                editBoxRectangles[3] = new Rectangle(241, 82, 246 - 241, 90 - 82);
                editBoxRectangles[4] = new Rectangle(8, 90, 14 - 8, 113 - 90);
                editBoxRectangles[5] = new Rectangle(241, 90, 246 - 241, 113 - 90);
                editBoxRectangles[6] = new Rectangle(8, 113, 14 - 8, 121 - 113);
                editBoxRectangles[7] = new Rectangle(14, 113, 241 - 14, 121 - 113);
                editBoxRectangles[8] = new Rectangle(241, 113, 246 - 241, 121 - 113);


                System.Resources.ResourceManager m = new System.Resources.ResourceManager("ScriptingSystem.Resources", typeof(GUIProperties).Assembly, null);
                object img = m.GetObject("dxutcontrols", null);

                controlsPicture = (Bitmap)img;// Image.FromFile(Application.StartupPath + "\\dxutcontrols.png");
            }
            catch
            {
                try
                {
                    OpenFileDialog fileDialog = new OpenFileDialog();
                    fileDialog.Filter = "Png file (*.png)|*.png";
                    if (fileDialog.ShowDialog() == DialogResult.OK)
                    {
                        controlsPicture = (Bitmap)Image.FromFile(fileDialog.FileName);
                    }
                }
                catch
                {
                    throw new System.IO.FileNotFoundException();
                }
            }
        }
    }
}
