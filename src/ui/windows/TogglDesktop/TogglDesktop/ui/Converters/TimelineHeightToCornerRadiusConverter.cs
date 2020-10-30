using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class TimelineHeightToCornerRadiusConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var roundBottom = true;
            if (bool.TryParse(parameter?.ToString(), out var param)) roundBottom = param;
            var height = value is double d ? d : 0;
            if (height >= 15) return new CornerRadius(8,8, roundBottom ? 8 : 0, roundBottom ? 8 : 0);
            if (height >= 6) return new CornerRadius(4, 4, roundBottom ? 4 : 0, roundBottom ? 4 : 0);
            return 0;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
