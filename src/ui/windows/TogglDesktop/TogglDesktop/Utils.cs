using System;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using System.Windows;
using TogglDesktop.WPF;
using Size = System.Drawing.Size;

namespace TogglDesktop
{
public static class Utils
{
    public static void LoadWindowLocation(Window mainWindow, EditViewPopup editPopup)
    {
        try
        {
            if (editPopup != null)
            {
                var size = Properties.Settings.Default.EditSize;
                editPopup.Width = size.Width;
                editPopup.Height = size.Height;
            }
            if (Properties.Settings.Default.Maximized)
            {
                mainWindow.WindowState = WindowState.Maximized;
            }
            else if (Properties.Settings.Default.Minimized)
            {
                mainWindow.WindowState = WindowState.Minimized;
            }

            long x = 0, y = 0, h = 0, w = 0;
            if (Toggl.WindowSettings(ref x, ref y, ref h, ref w))
            {
                mainWindow.Left = x;
                mainWindow.Top = y;

                if (h >= 0 && w >= 0)
                {
                    mainWindow.Width = w;
                    mainWindow.Height = h;
                }
            }

            if (!visibleOnAnyScreen(mainWindow))
            {
                var location = Screen.PrimaryScreen.WorkingArea.Location;
                mainWindow.Left = location.X;
                mainWindow.Top = location.Y;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("Could not load window location: ", ex);
        }
    }

    private static bool visibleOnAnyScreen(Window f)
    {
        var windowBounds = new Rectangle(
            (int)f.Left, (int)f.Top, (int)f.Width, (int)f.Height
            );

        return Screen.AllScreens
            .Any(s => s.WorkingArea.IntersectsWith(windowBounds));
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


    public struct KeyCombination
    {
        private readonly ModifierKeys modifiers;
        private readonly string keyCode;

        public KeyCombination(ModifierKeys modifiers, string keyCode)
        {
            this.modifiers = modifiers;
            this.keyCode = keyCode;
        }

        public ModifierKeys Modifiers {
            get {
                return this.modifiers;
            }
        }
        public string KeyCode {
            get {
                return this.keyCode;
            }
        }
    }

    public static void SetShortcutForShow(KeyCombination? e)
    {
        try
        {
            if (e.HasValue)
            {
                Properties.Settings.Default.ShowModifiers = e.Value.Modifiers;
                Properties.Settings.Default.ShowKey = e.Value.KeyCode;
            }
            else
            {
                Properties.Settings.Default.ShowModifiers = 0;
                Properties.Settings.Default.ShowKey = null;
            }

            Properties.Settings.Default.Save();
        }
        catch (Exception ex)
        {
            Console.WriteLine("Could not set shortcut for show: {0}", ex);
        }
    }

    public static void SetShortcutForStart(KeyCombination? e)
    {
        try
        {
            if (e.HasValue)
            {
                Properties.Settings.Default.StartModifiers = e.Value.Modifiers;
                Properties.Settings.Default.StartKey = e.Value.KeyCode;
            }
            else
            {
                Properties.Settings.Default.StartModifiers = 0;
                Properties.Settings.Default.StartKey = null;
            }

            Properties.Settings.Default.Save();
        }
        catch (Exception ex)
        {
            Console.WriteLine("Could not set shortcut for start: {0}", ex);
        }
    }

    public static void SaveWindowLocation(Window f, EditViewPopup edit)
    {
        try
        {
            Toggl.SetWindowSettings(
                (long)f.Left,
                (long)f.Top,
                (long)f.Height,
                (long)f.Width);

            if (f.WindowState == WindowState.Maximized)
            {
                Properties.Settings.Default.Maximized = true;
                Properties.Settings.Default.Minimized = false;
            }
            else if (f.WindowState == WindowState.Normal)
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
                Properties.Settings.Default.EditSize = new Size((int)edit.Width, (int)edit.Height);
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
