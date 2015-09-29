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
    #region window position loading and saving

    public static void LoadWindowLocation(Window mainWindow, EditViewPopup editPopup)
    {
        try
        {
            if (editPopup != null)
            {
                editPopup.Width = Toggl.GetEditViewWidth();
            }
            if (Toggl.GetWindowMaximized())
            {
                mainWindow.WindowState = WindowState.Maximized;
            }
            else if (Toggl.GetWindowMinimized())
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
            Toggl.SetWindowMaximized(state == WindowState.Maximized);
            Toggl.SetWindowMinimized(state == WindowState.Minimized);

            if (edit != null)
            {
                Toggl.SetEditViewWidth((long)edit.Width);
            }
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
        setShortcut(e, "show",
            Toggl.SetKeyModifierShow,
            Toggl.SetKeyShow
            );
    }

    public static void SetShortcutForStart(KeyCombination? e)
    {
        setShortcut(e, "start",
            Toggl.SetKeyModifierStart,
            Toggl.SetKeyStart
            );
    }

    private static void setShortcut(KeyCombination? e, string shortcutName,
                                    Action<ModifierKeys> setModifier, Action<string> setKey)
    {
        try
        {
            if (e.HasValue)
            {
                setModifier(e.Value.Modifiers);
                setKey(e.Value.KeyCode);
            }
            else
            {
                setModifier(0);
                setKey(null);
            }
        }
        catch (Exception ex)
        {
            Toggl.Debug(string.Format("Could not set shortcut for {0}: {1}", shortcutName, ex));
        }
    }

    #endregion
}
}
