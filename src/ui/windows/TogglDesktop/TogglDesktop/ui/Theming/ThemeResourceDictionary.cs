using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace TogglDesktop.Theming
{
    internal sealed class ThemeResourceDictionary : ResourceDictionary
    {
        private static readonly Dictionary<ThemeType, List<ThemeResourceDictionary>> loadedDictionaries
            = new Dictionary<ThemeType, List<ThemeResourceDictionary>>();

        private ThemeType _type;

        public ThemeType Type
        {
            get { return this._type; }
            set
            {
                if (this._type == value)
                    return;

                if (this._type != 0)
                    loadedDictionaries[this._type].Remove(this);

                this._type = value;
                this.addToLoadedThemes();
            }
        }

        private void addToLoadedThemes()
        {
            if (!loadedDictionaries.TryGetValue(this._type, out List<ThemeResourceDictionary> themes))
            {
                themes = new List<ThemeResourceDictionary>();
                loadedDictionaries.Add(this._type, themes);
            }

            themes.Add(this);
        }

        public static ThemeResourceDictionary Load(ThemeType type, string name)
        {
            var uri = new Uri(
                string.Format("ui/Resources/Themes/{0}/{1}.xaml", type, name),
                UriKind.Relative
                );

            ThemeResourceDictionary dictionary = null;

            if (loadedDictionaries.TryGetValue(type, out List<ThemeResourceDictionary> themes))
            {
                dictionary = themes.FirstOrDefault(d => d.Source == uri);
            }

            return dictionary
                ?? new ThemeResourceDictionary { Source = uri, Type = type };
        }
    }
}
