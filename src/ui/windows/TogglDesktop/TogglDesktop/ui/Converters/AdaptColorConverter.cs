using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace TogglDesktop.Converters
{
    public class AdaptColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var hex = value as string;
            var projectColorString = string.IsNullOrEmpty(hex) ? "999999" : (hex.StartsWith("#") ? hex.Substring(1) : hex);
            var rgbColor = Toggl.GetAdaptiveRgbColorFromHex(projectColorString,
                Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground);
            return Color.FromRgb((byte) rgbColor.r, (byte)rgbColor.g, (byte)rgbColor.b);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}