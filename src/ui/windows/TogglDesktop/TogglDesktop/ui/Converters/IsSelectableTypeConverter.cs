using System;
using System.Globalization;
using System.Windows.Data;
using TogglDesktop.AutoCompletion;

namespace TogglDesktop.Converters
{
    public class IsSelectableTypeConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var isNonSelectable = value is ItemType itemType && itemType < 0;
            return !isNonSelectable;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}