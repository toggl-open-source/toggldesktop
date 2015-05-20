using System.Windows.Forms;

namespace TogglDesktop
{
    class ControlHelper
    {
        public static bool NoFieldFocused(Control parent)
        {
            foreach (Control c in parent.Controls)
            {
                if (c.Focused)
                {
                    return false;
                }
                if (c.Controls.Count > 0)
                {
                    if (!NoFieldFocused(c))
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    }
}
