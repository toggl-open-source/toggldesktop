using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace TogglDesktop.Theming
{
    sealed class ThemeResourceDictionary : ResourceDictionary
    {
        private static readonly Dictionary<ThemeType,List<ThemeResourceDictionary>> loadedDictionaries
            = new Dictionary<ThemeType, List<ThemeResourceDictionary>>();

        private ThemeType type;

        public ThemeType Type
        {
            get { return this.type; }
            set
            {
                if (this.type == value)
                    return;

                if (this.type != 0)
                    loadedDictionaries[this.type].Remove(this);

                this.type = value;
                this.addToLoadedThemes();
            }
        }

        private void addToLoadedThemes()
        {
            List<ThemeResourceDictionary> themes;

            if (!loadedDictionaries.TryGetValue(this.type, out themes))
            {
                themes = new List<ThemeResourceDictionary>();
                loadedDictionaries.Add(this.type, themes);
            }

            themes.Add(this);
        }

        public static ThemeResourceDictionary Load(ThemeType type, string name)
        {
            var uri = new Uri(
                string.Format("ui/Resources/Themes/{0}/{1}.xaml", type, name),
                UriKind.Relative
                );

            List<ThemeResourceDictionary> themes;
            ThemeResourceDictionary dictionary = null;

            if (loadedDictionaries.TryGetValue(type, out themes))
            {
                dictionary = themes.FirstOrDefault(d => d.Source == uri);
            }

            return dictionary
                ?? new ThemeResourceDictionary { Source = uri, Type = type };
        }
    }
}