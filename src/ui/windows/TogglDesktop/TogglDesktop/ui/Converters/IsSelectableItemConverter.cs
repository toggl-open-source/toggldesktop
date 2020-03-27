using System;
using System.Globalization;
using System.Windows.Data;
using TogglDesktop.AutoCompletion.Items;

namespace TogglDesktop.Converters
{
    public class IsSelectableItemConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var isEnabled = value is AutoCompleteItem item && item.IsSelectable();
            return isEnabled;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}