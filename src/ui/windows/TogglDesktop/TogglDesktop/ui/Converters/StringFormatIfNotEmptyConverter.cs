using System;
using System.Globalization;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class StringFormatIfNotEmptyConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var originalValue = value as string;
            if (string.IsNullOrEmpty(originalValue))
            {
                return originalValue;
            }

            var formatString = parameter as string;
            if (string.IsNullOrEmpty(formatString))
            {
                return originalValue;
            }

            return string.Format(formatString, originalValue);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}