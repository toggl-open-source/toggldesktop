using System;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    class FalseToHiddenVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            var input = (null == parameter)
                ? (bool)value :
                !((bool)value);
            return input ? Visibility.Visible : Visibility.Hidden;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
