using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Media;
using Rectangle = System.Drawing.Rectangle;

namespace TogglDesktop
{
public static class Utils
{
    #region window position loading and saving

    public static void LoadWindowLocation(Window mainWindow, EditViewPopup editPopup, MiniTimerWindow miniTimer)
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

        if (miniTimer != null)
        {
            x = Toggl.GetMiniTimerX();
            y = Toggl.GetMiniTimerY();
            w = Toggl.GetMiniTimerW();
            miniTimer.Left = x;
            miniTimer.Top = y;
            miniTimer.Width = w;
            Toggl.Debug("Retrieved mini timer location ({0}x{1} by {2})", x, y, w);

            if (!visibleOnAnyScreen(miniTimer))
            {
                var location = Screen.PrimaryScreen.WorkingArea.Location;
                miniTimer.Left = location.X;
                miniTimer.Top = location.Y;
                Toggl.Debug("Force moved mini timer to primary screen");
            }
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

    public static void SaveWindowLocation(Window mainWindow, EditViewPopup edit, MiniTimerWindow miniTimer)
    {
        long x, y, w, h;

        if (mainWindow.WindowState == WindowState.Minimized)
        {
            var rb = mainWindow.RestoreBounds;
            x = (long)rb.X;
            y = (long)rb.Y;
            w = (long)rb.Width;
            h = (long)rb.Height;
        }
        else
        {

            x = (long)mainWindow.Left;
            y = (long)mainWindow.Top;
            w = (long)mainWindow.Width;
            h = (long)mainWindow.Height;
        }

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

        if (miniTimer != null)
        {
            x = (long)miniTimer.Left;
            y = (long)miniTimer.Top;
            w = (long)miniTimer.Width;
            Toggl.SetMiniTimerX(x);
            Toggl.SetMiniTimerY(y);
            Toggl.SetMiniTimerW(w);
            Toggl.Debug("Saved mini timer location ({0}x{1} by {2})", x, y, w);
        }
    }

    #endregion

    #region keyboard shortcuts

    public struct KeyCombination : IEquatable<KeyCombination>
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

        public bool Equals(KeyCombination other)
        {
            return this.modifiers == other.modifiers && string.Equals(this.keyCode, other.keyCode);
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            return obj is KeyCombination && Equals((KeyCombination)obj);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                return ((int)this.modifiers * 397) ^ (this.keyCode != null ? this.keyCode.GetHashCode() : 0);
            }
        }

        public static bool operator ==(KeyCombination c0, KeyCombination c1)
        {
            return c0.Equals(c1);
        }

        public static bool operator !=(KeyCombination c0, KeyCombination c1)
        {
            return !(c0 == c1);
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
