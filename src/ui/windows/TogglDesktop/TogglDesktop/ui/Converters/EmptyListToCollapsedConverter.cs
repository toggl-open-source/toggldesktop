using System;
using System.Collections;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class EmptyListToCollapsedConverter : IValueConverter
    {
        public bool Inverse { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var visibility = Visibility.Collapsed;
            if (value is IList list && list.Count > 0)
                visibility = Visibility.Visible;
            if (Inverse) visibility = visibility == Visibility.Visible ? Visibility.Collapsed : Visibility.Visible;
            return visibility;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
