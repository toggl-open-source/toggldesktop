using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Media;
using Microsoft.Win32;
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
        if (Toggl.WindowSettings(ref x, ref y, ref h, ref w)
            && ValidateWindowSettings(x, y, h, w))
        {
            mainWindow.Left = x;
            mainWindow.Top = y;
            mainWindow.Width = w;
            mainWindow.Height = h;
            Toggl.Debug("Retrieved window location and size ({0}x{1} by {2}x{3})", x, y, w, h);
        }
        else
        {
            mainWindow.Width = 330;
            mainWindow.Height = 510;
            Toggl.Debug("Failed to retrieve window location and size. Setting the default size.");
        }

        // First try to shift the window onto the bounding box of visible screens
        if (ShiftWindowOntoVisibleArea(mainWindow))
        {
            Toggl.Debug("Shifted main window onto visible area");
        }
        // Then handle the case where the window is in the bounding box but not on any of the screens
        if (!VisibleOnAnyScreen(mainWindow))
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
            if (ValidateMiniTimerWindowSettings(x, y, w))
            {
                miniTimer.Left = x;
                miniTimer.Top = y;
                miniTimer.Width = w;
                Toggl.Debug("Retrieved mini timer location ({0}x{1} by {2})", x, y, w);
            }
            else
            {
                const int defaultX = 0;
                const int defaultY = 0;
                const int defaultWidth = 360;
                miniTimer.Left = defaultX;
                miniTimer.Top = defaultY;
                miniTimer.Width = defaultWidth;
                Toggl.Debug($"Set default mini-timer position and size: ({defaultX}x{defaultY} by {defaultWidth}");
            }

            CheckMinitimerVisibility(miniTimer);
        }
    }

    private static bool ValidateWindowSettings(long x, long y, long h, long w)
    {
        return new[] {x, y, h, w}.All(v => v >= int.MinValue && v <= int.MaxValue) && h > 0 && w > 0;
    }

    private static bool ValidateMiniTimerWindowSettings(long x, long y, long w)
    {
        return new[] {x, y, w}.All(v => v >= int.MinValue && v <= int.MaxValue) && w > 0;
    }

    public static void CheckMinitimerVisibility(MiniTimerWindow miniTimer)
    {
        // First try to shift the window onto the bounding box of visible screens
        if (ShiftWindowOntoVisibleArea(miniTimer))
        {
            Toggl.Debug("Shifted mini timer onto visible area");
        }

        // Then handle the case where the window is in the bounding box but not on any of the screens
        if (!VisibleOnAnyScreen(miniTimer))
        {
            var location = Screen.PrimaryScreen.WorkingArea.Location;
            miniTimer.Left = location.X;
            miniTimer.Top = location.Y;
            Toggl.Debug("Force moved mini timer to primary screen");
        }
    }

    private static bool ShiftWindowOntoVisibleArea(Window window)
    {
        var shifted = false;
        if (window.Top < SystemParameters.VirtualScreenTop)
        {
            window.Top = SystemParameters.VirtualScreenTop;
            shifted = true;
        }

        if (window.Left < SystemParameters.VirtualScreenLeft)
        {
            window.Left = SystemParameters.VirtualScreenLeft;
            shifted = true;
        }

        if (window.Left + window.Width > SystemParameters.VirtualScreenLeft + SystemParameters.VirtualScreenWidth)
        {
            window.Left = SystemParameters.VirtualScreenWidth + SystemParameters.VirtualScreenLeft - window.Width;
            shifted = true;
        }

        if (window.Top + window.Height > SystemParameters.VirtualScreenTop + SystemParameters.VirtualScreenHeight)
        {
            window.Top = SystemParameters.VirtualScreenHeight + SystemParameters.VirtualScreenTop - window.Height;
            shifted = true;
        }

        return shifted;
    }

    private static bool VisibleOnAnyScreen(Window f)
    {
        var scalingRatio = Math.Max(Screen.PrimaryScreen.WorkingArea.Width / SystemParameters.PrimaryScreenWidth,
            Screen.PrimaryScreen.WorkingArea.Height / SystemParameters.PrimaryScreenHeight);

        var windowBounds = new Rectangle((int) (f.Left * scalingRatio), (int) (f.Top * scalingRatio),
            (int) (f.Width * scalingRatio), (int) (f.Height * scalingRatio));

        return Screen.AllScreens.Any(s => s.WorkingArea.IntersectsWith(windowBounds));
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

        var success = ValidateWindowSettings(x, y, h, w)
                      && Toggl.SetWindowSettings(x, y, h, w);

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

        #region registry

        private const string StartupAppsRegistryPath = @"Software\Microsoft\Windows\CurrentVersion\Run";
        public static bool GetLaunchOnStartupRegistry()
        {
            var subKey = Registry.CurrentUser.OpenSubKey(StartupAppsRegistryPath);
            return subKey?.GetValue("TogglDesktop") != null;
        }

        public static void SaveLaunchOnStartupRegistry(bool launchOnStartup)
        {
            if (GetLaunchOnStartupRegistry() == launchOnStartup)
            {
                return;
            }

            using (var subKey = Registry.CurrentUser.OpenSubKey(StartupAppsRegistryPath, true)
                         ?? Registry.CurrentUser.CreateSubKey(StartupAppsRegistryPath))
            {
                if (subKey == null)
                {
                    throw new ApplicationException($"Failed to open or create current user registry key: {StartupAppsRegistryPath}");
                }

                if (launchOnStartup)
                {
                    subKey.SetValue("TogglDesktop", "\"" + Assembly.GetEntryAssembly().Location + "\"" + " --minimize");
                }
                else
                {
                    subKey.DeleteValue("TogglDesktop");
                }
            }
        }

        public static bool TryOpenInDefaultBrowser(string url)
        {
            return tryOpenInDefaultBrowserFromRegistry(url)
                   || tryOpenInBuiltInBrowser(url);
        }

        private static bool tryOpenInDefaultBrowserFromRegistry(string url)
        {
            try
            {
                using (var browserKey = Registry.CurrentUser.OpenSubKey(@"Software\Microsoft\Windows\Shell\Associations\UrlAssociations\http\UserChoice", false))
                {
                    if (browserKey == null) return false;
                    var browserKeyName = browserKey.GetValue("ProgId") as string;
                    if (string.IsNullOrEmpty(browserKeyName)) return false;
                    using (var openBrowserCmdKey = Registry.ClassesRoot.OpenSubKey(browserKeyName + @"\shell\open\command"))
                    {
                        if (openBrowserCmdKey == null) return false;
                        var openBrowserCmdValue = openBrowserCmdKey.GetValue(null) as string;
                        if (string.IsNullOrEmpty(openBrowserCmdValue)) return false;
                        openBrowserCmdValue = openBrowserCmdValue.Replace("%1", url);
                        var splitResult = openBrowserCmdValue.SplitByWhiteSpaceUnlessEnclosedInQuotes();
                        var fileName = splitResult.First(); // take the file name
                        var args = string.Join(" ", splitResult.Skip(1)); // merge the rest of args back
                        Process.Start(fileName, args);
                        return true;
                    }
                }
            }
            catch
            {
                return false;
            }
        }

        private static bool tryOpenInBuiltInBrowser(string url)
        {
            try
            {
                if (Environment.OSVersion.Version.Major >= 10)
                {
                    Process.Start(@"C:\WINDOWS\system32\LaunchWinApp.exe", url);
                }
                else
                {
                    Process.Start("iexplore.exe", url);
                }

                return true;
            }
            catch
            {
                return false;
            }
        }

        #endregion

        #region file system

        public static void DeleteFile(string fullPath)
        {
            File.SetAttributes(fullPath, FileAttributes.Normal);
            File.Delete(fullPath);
        }

        #endregion file system

        #region environment

        public static string Bitness() => Environment.Is64BitProcess ? "(64-bit)" : "(32-bit)";

        #endregion environment
}
}
