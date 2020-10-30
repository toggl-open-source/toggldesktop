using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;
using TogglDesktop.Theming;

namespace TogglDesktop.Converters
{
    public class BrushOpacityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (parameter is double opacity)
            {
                var hex = value as string;
                var brush = Utils.AdaptedProjectColorBrushFromString(hex, Theme.ShapeColorAdaptation.Value);
                var newBrush = new SolidColorBrush(brush.Color)
                {
                    Opacity = opacity
                };
                newBrush.Freeze();
                return newBrush;
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
