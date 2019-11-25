using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace TogglDesktop.Converters
{
    class ColorsToBrushConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            foreach (var val in values)
            {
                switch (val)
                {
                    case Brush brush:
                        return brush;
                    case string str when !string.IsNullOrEmpty(str):
                        return (SolidColorBrush)(new BrushConverter().ConvertFrom(str));
                }
            }

            return null;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
