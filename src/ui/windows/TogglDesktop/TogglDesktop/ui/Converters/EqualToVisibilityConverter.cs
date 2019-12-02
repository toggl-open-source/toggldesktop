using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class EqualToVisibilityConverter : IValueConverter
    {
        public object ExpectedValue { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return Equals(value, ExpectedValue) ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

    }
}