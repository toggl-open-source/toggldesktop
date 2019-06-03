using System;
using System.Globalization;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class ElementSizeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var percentage = double.Parse(parameter.ToString());
            return double.Parse(value.ToString()) * percentage;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}