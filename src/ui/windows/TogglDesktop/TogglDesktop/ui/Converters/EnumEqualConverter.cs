using System;
using System.Globalization;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class EnumEqualConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value == null || parameter == null || !value.GetType().IsEnum) return false;
            return Enum.Equals(value, parameter);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}