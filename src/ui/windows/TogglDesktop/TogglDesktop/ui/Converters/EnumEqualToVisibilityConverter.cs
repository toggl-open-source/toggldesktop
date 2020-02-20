using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class EnumEqualToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value == null || parameter == null || !value.GetType().IsEnum) return false;
            return Enum.Equals(value, parameter) ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

}