using System;
using System.Globalization;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class RectangleHeightToRadiusConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (parameter == null || !(value is double)) return 0;
            var height = (double)value;
            if (height >= 15) return parameter.ToString() == "RadiusX" ? 15 : 7;
            if (height >= 6) return parameter.ToString() == "RadiusX" ? 4 : 15;
            return 0;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
