using System;
using System.Globalization;
using System.Windows.Data;
using TogglDesktop.Theming;

namespace TogglDesktop.Converters
{
    public class AdaptProjectColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var hex = value as string;
            var opacity = parameter is double val && val >= 0 && val <= 1 ? val : 1.0;
            return Utils.AdaptedProjectColorBrushFromString(hex, Theme.ShapeColorAdaptation.Value, opacity);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class AdaptProjectTextColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var hex = value as string;
            return Utils.AdaptedProjectColorBrushFromString(hex, Theme.TextColorAdaptation.Value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}