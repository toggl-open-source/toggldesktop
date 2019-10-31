using System;
using System.Linq;
using System.Windows;
using Microsoft.Win32;

namespace TogglDesktop.Theming
{
    static class Theme
    {
        /// <returns>Activated color scheme.</returns>
        public static ColorScheme ActivateDetectedColorSchemeOrDefault(ColorScheme defaultColorScheme = ColorScheme.Light)
        {
            var colorScheme = DetectOsColorScheme().GetValueOrDefault(defaultColorScheme);
            ActivateColorScheme(colorScheme);
            return colorScheme;
        }

        public static void ActivateColorScheme(ColorScheme colorScheme)
        {
            Activate(ThemeType.ColorScheme, colorScheme.ToString());
        }

        private static void Activate(ThemeType type, string name)
        {
            var app = Application.Current;

            var mergedDictionaries = app.Resources.MergedDictionaries;

            var currentTheme = mergedDictionaries
                .OfType<ThemeResourceDictionary>()
                .FirstOrDefault(d => d.Type == type);

            var newTheme = ThemeResourceDictionary.Load(type, name);

            if (currentTheme == newTheme)
                return;

            if (newTheme != null)
                mergedDictionaries.Add(newTheme);

            if (currentTheme != null)
                mergedDictionaries.Remove(currentTheme);
        }

        private static ColorScheme? DetectOsColorScheme()
        {
            try
            {
                using (var subKey = Registry.CurrentUser.OpenSubKey(
                    @"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize", false))
                {
                    if (subKey == null)
                    {
                        return null;
                    }

                    var value = Convert.ToInt32(subKey.GetValue("AppsUseLightTheme", -1));
                    switch (value)
                    {
                        case 0: return ColorScheme.Dark;
                        case 1: return ColorScheme.Light;
                        default: return null;
                    }
                }
            }
            catch (Exception e)
            {
                BugsnagService.NotifyBugsnag(e);
                return null;
            }
        }
    }
}