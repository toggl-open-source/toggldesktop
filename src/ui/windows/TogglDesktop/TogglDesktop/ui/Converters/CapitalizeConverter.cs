using System;
using System.Globalization;
using System.Linq;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class CapitalizeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var stringValue = value as string;
            if (string.IsNullOrWhiteSpace(stringValue))
            {
                return value;
            }

            return char.ToUpper(stringValue[0]) + stringValue.Substring(1);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}