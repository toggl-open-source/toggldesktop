
using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class HasAnyFlagToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value == null || parameter == null || !value.GetType().IsEnum || !value.GetType().IsEnum) return false;
            return ((int)value & (int)parameter) != 0 ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
