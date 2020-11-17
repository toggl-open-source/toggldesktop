using System;
using System.Globalization;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class CalculateCenterConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is double totalWidth && parameter is double elementWidth)
            {
                return (totalWidth - elementWidth) / 2;
            }

            return 0;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
