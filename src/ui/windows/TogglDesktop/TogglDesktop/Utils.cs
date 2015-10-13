using System;
using System.Linq;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Media;
using TogglDesktop.WPF;
using Rectangle = System.Drawing.Rectangle;

namespace TogglDesktop
{
public static class Utils
{
    #region window position loading and saving

    public static void LoadWindowLocation(Window mainWindow, EditViewPopup editPopup)
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
            mainWindow.Width = w;
            mainWindow.Height = h;
            Toggl.Debug("Retrieved window location and size ({0}x{1} by {2}x{3})", x, y, w, h);
        }
        else
        {
            Toggl.Debug("Failed to retrieve window location and size");
        }

        if (!visibleOnAnyScreen(mainWindow))
        {
            var location = Screen.PrimaryScreen.WorkingArea.Location;
            mainWindow.Left = location.X;
            mainWindow.Top = location.Y;
            Toggl.Debug("Force moved window to primary screen");
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
        var x = (long)mainWindow.Left;
        var y = (long)mainWindow.Top;
        var w = (long)mainWindow.Width;
        var h = (long)mainWindow.Height;

        var success = Toggl.SetWindowSettings(x, y, h, w);

        Toggl.Debug(success
                    ? "Saved window location and size ({0}x{1} by {2}x{3})"
                    : "Failed to save window location and size ({0}x{1} by {2}x{3})",
                    x, y, w, h);

        var state = mainWindow.WindowState;
        Toggl.SetWindowMaximized(state == WindowState.Maximized);
        Toggl.SetWindowMinimized(state == WindowState.Minimized);

        if (edit != null)
        {
            Toggl.SetEditViewWidth((long)edit.Width);
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

    #region project color conversion

    public static Color ProjectColorFromString(string rgb)
    {
        var projectColourString = string.IsNullOrEmpty(rgb) ? "#999999" : rgb;
        var projectColor = (Color)(ColorConverter.ConvertFromString(projectColourString) ?? Color.FromRgb(153, 153, 153));
        return projectColor;
    }

    public static SolidColorBrush ProjectColorBrushFromString(string rgb)
    {
        var color = ProjectColorFromString(rgb);
        var brush = new SolidColorBrush(color);
        brush.Freeze();
        return brush;
    }

    #endregion
}
}
