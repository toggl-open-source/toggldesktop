using System;
using System.Globalization;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class StringPrependConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var originalValue = value as string;
            if (string.IsNullOrEmpty(originalValue))
            {
                return originalValue;
            }

            var prefix = (parameter as string) ?? string.Empty;
            return prefix + originalValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}