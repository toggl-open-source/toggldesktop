using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TogglDesktop
{
    public static class Utils
    {
        public static void LoadWindowLocation(Form f)
        {
            if (Properties.Settings.Default.Maximized)
            {
                f.WindowState = FormWindowState.Maximized;
                f.Location = Properties.Settings.Default.Location;
                f.Size = Properties.Settings.Default.Size;
            }
            else if (Properties.Settings.Default.Minimized)
            {
                f.WindowState = FormWindowState.Minimized;
                f.Location = Properties.Settings.Default.Location;
                f.Size = Properties.Settings.Default.Size;
            }
            else
            {
                f.Location = Properties.Settings.Default.Location;
                f.Size = Properties.Settings.Default.Size;
            }
        }

        public static void SaveWindowLocation(Form f)
        {
            if (f.WindowState == FormWindowState.Maximized)
            {
                Properties.Settings.Default.Location = f.RestoreBounds.Location;
                Properties.Settings.Default.Size = f.RestoreBounds.Size;
                Properties.Settings.Default.Maximized = true;
                Properties.Settings.Default.Minimized = false;
            }
            else if (f.WindowState == FormWindowState.Normal)
            {
                Properties.Settings.Default.Location = f.Location;
                Properties.Settings.Default.Size = f.Size;
                Properties.Settings.Default.Maximized = false;
                Properties.Settings.Default.Minimized = false;
            }
            else
            {
                Properties.Settings.Default.Location = f.RestoreBounds.Location;
                Properties.Settings.Default.Size = f.RestoreBounds.Size;
                Properties.Settings.Default.Maximized = false;
                Properties.Settings.Default.Minimized = true;
            }
            Properties.Settings.Default.Save();
        }
    }
}
