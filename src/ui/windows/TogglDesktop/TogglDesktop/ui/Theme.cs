using System.Linq;
using System.Windows;

namespace TogglDesktop
{
    static class Theme
    {
        public static void Activate(ThemeTypes type, string name)
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
    }
}