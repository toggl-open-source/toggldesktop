using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace TogglDesktop.Converters
{
    class StringToBrushConverter : IValueConverter
    {
        public Brush OnNullOrEmpty { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var inputString = value as string;
            return string.IsNullOrEmpty(inputString)
                ? OnNullOrEmpty
                : (SolidColorBrush)(new BrushConverter().ConvertFrom(inputString));
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
