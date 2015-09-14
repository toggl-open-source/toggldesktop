using System;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using System.Windows;
using TogglDesktop.Properties;
using TogglDesktop.WPF;
using Size = System.Drawing.Size;

namespace TogglDesktop
{
public static class Utils
{
    #region window position loading and saving

    public static void LoadWindowLocation(Window mainWindow, EditViewPopup editPopup)
    {
        try
        {
            if (editPopup != null)
            {
                var size = Settings.Default.EditSize;
                editPopup.Width = size.Width;
                editPopup.Height = size.Height;
            }
            if (Settings.Default.Maximized)
            {
                mainWindow.WindowState = WindowState.Maximized;
            }
            else if (Settings.Default.Minimized)
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
            Toggl.Debug("Could not load window location: " + ex);
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

    public static void SaveWindowLocation(Window mainWindow, EditViewPopup edit)
    {
        try
        {
            Toggl.SetWindowSettings(
                (long)mainWindow.Left,
                (long)mainWindow.Top,
                (long)mainWindow.Height,
                (long)mainWindow.Width);

            var state = mainWindow.WindowState;
            Settings.Default.Maximized = state == WindowState.Maximized;
            Settings.Default.Minimized = state == WindowState.Minimized;

            if (edit != null)
            {
                Settings.Default.EditSize = new Size((int)edit.Width, (int)edit.Height);
            }

            Settings.Default.Save();
        }
        catch (Exception ex)
        {
            Toggl.Debug("Could not save window location: " + ex);
        }
    }

    #endregion

    #region keyboard shortcuts
    
    public struct KeyCombination
    {
        private readonly ModifierKeys modifiers;
        private readonly string keyCode;

        public KeyCombination(ModifierKeys modifiers, string keyCode)
        {
            this.modifiers = modifiers;
            this.keyCode = keyCode;
        }

        public ModifierKeys Modifiers { get { return this.modifiers; } }
        public string KeyCode { get { return this.keyCode; } }
    }

    public static void SetShortcutForShow(KeyCombination? e)
    {
        setShortcut(e, "show",
            (s, m) => s.ShowModifiers = m,
            (s, k) => s.ShowKey = k
            );
    }

    public static void SetShortcutForStart(KeyCombination? e)
    {
        setShortcut(e, "start",
            (s, m) => s.StartModifiers = m,
            (s, k) => s.StartKey = k
            );
    }

    private static void setShortcut(KeyCombination? e, string shortcutName,
        Action<Settings, ModifierKeys> setModifier, Action<Settings, string> setKey)
    {
        try
        {
            if (e.HasValue)
            {
                setModifier(Settings.Default, e.Value.Modifiers);
                setKey(Settings.Default, e.Value.KeyCode);
            }
            else
            {
                setModifier(Settings.Default, 0);
                setKey(Settings.Default, null);
            }

            Settings.Default.Save();
        }
        catch (Exception ex)
        {
            Toggl.Debug(string.Format("Could not set shortcut for {0}: {1}", shortcutName, ex));
        }
    }

    #endregion
}
}
