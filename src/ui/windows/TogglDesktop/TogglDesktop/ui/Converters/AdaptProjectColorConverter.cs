using System;
using System.Globalization;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Windows.Data;
using TogglDesktop.Theming;

namespace TogglDesktop.Converters
{
    public class AdaptProjectColorConverter : IValueConverter
    {
        public static readonly BehaviorSubject<Toggl.TogglAdaptiveColor> AdaptationType =
            new BehaviorSubject<Toggl.TogglAdaptiveColor>(Toggl.TogglAdaptiveColor.AdaptiveColorShapeOnLightBackground);

        static AdaptProjectColorConverter()
        {
            Theme.CurrentColorScheme.Select(ColorSchemeExtensions.ToAdaptiveShapeColor).Subscribe(AdaptationType.OnNext);
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var hex = value as string;
            return Utils.AdaptedProjectColorBrushFromString(hex, AdaptationType.Value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class AdaptProjectTextColorConverter : IValueConverter
    {
        public static readonly BehaviorSubject<Toggl.TogglAdaptiveColor> AdaptationType =
            new BehaviorSubject<Toggl.TogglAdaptiveColor>(Toggl.TogglAdaptiveColor.AdaptiveColorTextOnLightBackground);

        static AdaptProjectTextColorConverter()
        {
            Theme.CurrentColorScheme.Select(ColorSchemeExtensions.ToAdaptiveTextColor).Subscribe(AdaptationType.OnNext);        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var hex = value as string;
            return Utils.AdaptedProjectColorBrushFromString(hex, AdaptationType.Value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}