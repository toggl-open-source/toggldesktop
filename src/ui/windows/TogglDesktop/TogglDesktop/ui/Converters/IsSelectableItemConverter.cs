using System;
using System.Globalization;
using System.Windows.Data;
using TogglDesktop.AutoCompletion;

namespace TogglDesktop.Converters
{
    public class IsSelectableItemConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var isEnabled = value is ListBoxItemViewModel item && item.IsSelectable;
            return isEnabled;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}