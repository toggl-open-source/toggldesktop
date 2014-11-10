using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;

namespace TogglDesktop
{
    public static class Utils
    {
        public static void LoadWindowLocation(Form f, Form edit)
        {
            Size defaultMinimumSize = f.MinimumSize;
            Size defaultEditMinimumSize = edit.MinimumSize;
            edit.MinimumSize = Properties.Settings.Default.EditSize;
            edit.MinimumSize = defaultEditMinimumSize;
            if (Properties.Settings.Default.Maximized)
            {
                f.WindowState = FormWindowState.Maximized;
            }
            else if (Properties.Settings.Default.Minimized)
            {
                f.WindowState = FormWindowState.Minimized;
            }
            f.Location = Properties.Settings.Default.Location;
            f.Size = Properties.Settings.Default.Size;
            f.MinimumSize = Properties.Settings.Default.Size;
            f.MinimumSize = defaultMinimumSize;
            if (!visibleOnAnyScreen(f))
            {
                f.Location = Screen.PrimaryScreen.WorkingArea.Location;
            }
        }

        private static bool visibleOnAnyScreen(Form f)
        {
            foreach (Screen s in Screen.AllScreens)
            {
                if (s.WorkingArea.IntersectsWith(f.DesktopBounds))
                {
                    return true;
                }
            }
            return false;
        }

        public static void GetShortcutForShow(
            ref bool alt,
            ref bool ctrl,
            ref bool shift,
            ref string keyCode)
        {
            alt = Properties.Settings.Default.ShowAlt;
            ctrl = Properties.Settings.Default.ShowCtrl;
            shift = Properties.Settings.Default.ShowShift;
            keyCode = Properties.Settings.Default.ShowKey;
        }

        public static void GetShortcutForStart(
            ref bool alt,
            ref bool ctrl,
            ref bool shift,
            ref string keyCode)
        {
            alt = Properties.Settings.Default.StartAlt;
            ctrl = Properties.Settings.Default.StartCtrl;
            shift = Properties.Settings.Default.StartShift;
            keyCode = Properties.Settings.Default.StartKey;
        }

        public static void SetShortcutForShow(KeyEventArgs e)
        {
            if (e != null)
            {
                Properties.Settings.Default.ShowAlt = e.Alt;
                Properties.Settings.Default.ShowCtrl = e.Control;
                Properties.Settings.Default.ShowShift = e.Shift;
                Properties.Settings.Default.ShowKey = e.KeyCode.ToString();
            }
            else
            {
                Properties.Settings.Default.ShowAlt = false;
                Properties.Settings.Default.ShowCtrl = false;
                Properties.Settings.Default.ShowShift = false;
                Properties.Settings.Default.ShowKey = null;
            }
            Properties.Settings.Default.Save();
        }

        public static void SetShortcutForStart(KeyEventArgs e)
        {
            if (e != null)
            {
                Properties.Settings.Default.StartAlt = e.Alt;
                Properties.Settings.Default.StartCtrl = e.Control;
                Properties.Settings.Default.StartShift = e.Shift;
                Properties.Settings.Default.StartKey = e.KeyCode.ToString();
            }
            else
            {
                Properties.Settings.Default.StartAlt = false;
                Properties.Settings.Default.StartCtrl = false;
                Properties.Settings.Default.StartShift = false;
                Properties.Settings.Default.StartKey = null;
            }
            Properties.Settings.Default.Save();
        }

        public static void SaveWindowLocation(Form f, Form edit)
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
            Properties.Settings.Default.EditSize = edit.Size;
            Properties.Settings.Default.Save();
        }
    }
}
