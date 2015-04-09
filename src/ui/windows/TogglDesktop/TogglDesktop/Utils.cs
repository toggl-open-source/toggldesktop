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
            try
            {
                if (edit != null) {
                    edit.MinimumSize = Properties.Settings.Default.EditSize;
                }
                if (Properties.Settings.Default.Maximized)
                {
                    f.WindowState = FormWindowState.Maximized;
                }
                else if (Properties.Settings.Default.Minimized)
                {
                    f.WindowState = FormWindowState.Minimized;
                }

                Int64 x = 0, y = 0, h = 0, w = 0;
                if (Toggl.WindowSettings(ref x, ref y, ref h, ref w))
                {
                    if (x >= 0 && y >= 0)
                    {
                        f.Location = new Point((int)x, (int)y);
                    }

                    if (h >= 0 && w >= 0)
                    {
                        f.Size = new Size((int)w, (int)h);
                        f.MinimumSize = new Size((int)w, (int)h);
                    }
                }

                if (!visibleOnAnyScreen(f))
                {
                    f.Location = Screen.PrimaryScreen.WorkingArea.Location;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not load window location: ", ex);
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
            try
            {
                Properties.Settings.Default.ShowModifiers = GetModifiers(e);
                Properties.Settings.Default.ShowKey = GetKeyCode(e);
                Properties.Settings.Default.Save();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not set shortcut for show: ", ex);
            }
        }

        public static void SetShortcutForStart(KeyEventArgs e)
        {
            try
            {
                Properties.Settings.Default.StartModifiers = GetModifiers(e);
                Properties.Settings.Default.StartKey = GetKeyCode(e);
                Properties.Settings.Default.Save();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not set shortcut for start: ", ex);
            }
        }

        public static void SaveWindowLocation(Form f, Form edit)
        {
            try
            {
                Toggl.SetWindowSettings(
                    f.Location.X,
                    f.Location.Y,
                    f.Size.Height,
                    f.Size.Width);

                if (f.WindowState == FormWindowState.Maximized)
                {
                    Properties.Settings.Default.Maximized = true;
                    Properties.Settings.Default.Minimized = false;
                }
                else if (f.WindowState == FormWindowState.Normal)
                {
                    Properties.Settings.Default.Maximized = false;
                    Properties.Settings.Default.Minimized = false;
                }
                else
                {
                    Properties.Settings.Default.Maximized = false;
                    Properties.Settings.Default.Minimized = true;
                }

                if (edit != null) {
                    Properties.Settings.Default.EditSize = edit.Size;
                }

                Properties.Settings.Default.Save();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not save window location: ", ex);
            }
        }
    }
}
