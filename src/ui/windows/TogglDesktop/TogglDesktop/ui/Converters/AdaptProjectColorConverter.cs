using System;
using System.Globalization;
using System.Windows.Data;
using TogglDesktop.Theming;

namespace TogglDesktop.Converters
{
    public class AdaptProjectColorConverter : IValueConverter
    {
        private Toggl.TogglAdaptiveColor adaptationType = Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground;

        public AdaptProjectColorConverter()
        {
            Theme.CurrentColorScheme.Subscribe(colorScheme =>
            {
                adaptationType = colorScheme switch
                {
                    ColorScheme.Dark => Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnDarkBackground,
                    _ => Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground
                };
            });
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var hex = value as string;
            return Utils.AdaptedProjectColorBrushFromString(hex, adaptationType);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class AdaptProjectTextColorConverter : IValueConverter
    {
        private Toggl.TogglAdaptiveColor adaptationType = Toggl.TogglAdaptiveColor.AdaptiveColorTextOnLightBackground;

        public AdaptProjectTextColorConverter()
        {
            Theme.CurrentColorScheme.Subscribe(colorScheme =>
            {
                adaptationType = colorScheme switch
                {
                    ColorScheme.Dark => Toggl.TogglAdaptiveColor.AdaptiveColorTextOnDarkBackground,
                    _ => Toggl.TogglAdaptiveColor.AdaptiveColorTextOnLightBackground
                };
            });
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var hex = value as string;
            return Utils.AdaptedProjectColorBrushFromString(hex, adaptationType);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}