using System;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class BooleanInvertToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            var input = (value is bool)
                ? !(bool)value 
                : false;
            return input ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
