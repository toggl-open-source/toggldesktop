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

        public static TogglDesktop.ModifierKeys GetModifiers(KeyEventArgs e)
        {
            TogglDesktop.ModifierKeys modifiers = 0;
            if (e != null)
            {
                if (e.Alt)
                {
                    modifiers = modifiers | TogglDesktop.ModifierKeys.Alt;
                }
                if (e.Control)
                {
                    modifiers = modifiers | TogglDesktop.ModifierKeys.Control;
                }
                if (e.Shift)
                {
                    modifiers = modifiers | TogglDesktop.ModifierKeys.Shift;
                }
            }
            return modifiers;
        }

        public static string GetKeyCode(KeyEventArgs e)
        {
            if (e != null)
            {
                return e.KeyCode.ToString();
            }
            return null;
        }

        public static void SetShortcutForShow(KeyEventArgs e)
        {
            Properties.Settings.Default.ShowModifiers = GetModifiers(e);
            Properties.Settings.Default.ShowKey = GetKeyCode(e);
            Properties.Settings.Default.Save();
        }

        public static void SetShortcutForStart(KeyEventArgs e)
        {
            Properties.Settings.Default.StartModifiers = GetModifiers(e);
            Properties.Settings.Default.StartKey = GetKeyCode(e);
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
