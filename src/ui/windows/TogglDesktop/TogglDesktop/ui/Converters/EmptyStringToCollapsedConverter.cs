using System;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class EmptyStringToCollapsedConverter : IValueConverter
    {
        public bool Inverse { get; set; }
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            var stringValue = value as string;
            return (string.IsNullOrEmpty(stringValue) ^ Inverse)
                ? Visibility.Collapsed
                : Visibility.Visible;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}