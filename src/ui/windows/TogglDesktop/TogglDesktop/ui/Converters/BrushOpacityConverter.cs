using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace TogglDesktop.Converters
{
    public class BrushOpacityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is SolidColorBrush brush && parameter is double opacity)
            {
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
