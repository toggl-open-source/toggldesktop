using System;

namespace TogglDesktop
{
    class NonFlickerPanel : System.Windows.Forms.Panel
    {
        public NonFlickerPanel()
        {
            SetStyle(
                System.Windows.Forms.ControlStyles.UserPaint |
                System.Windows.Forms.ControlStyles.AllPaintingInWmPaint |
                System.Windows.Forms.ControlStyles.OptimizedDoubleBuffer,
                true);
        }
    }
}
