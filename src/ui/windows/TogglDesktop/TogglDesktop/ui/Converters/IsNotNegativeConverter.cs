using System;
using System.Globalization;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class IsNotNegativeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var isNegative = value is int integerValue && integerValue < 0;
            return !isNegative;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}