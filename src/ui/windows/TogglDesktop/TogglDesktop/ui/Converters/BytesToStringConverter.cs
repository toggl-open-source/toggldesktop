using System;
using System.Globalization;
using System.Windows.Data;
using static System.Convert;
using static System.Math;

namespace TogglDesktop.Converters
{
    public class BytesToStringConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var byteCount = (long)value;
            var humanReadableBytes = BytesToString(byteCount);
            return humanReadableBytes;
        }

        private static string BytesToString(long byteCount)
        {
            string[] suf = { "B", "KB", "MB", "GB", "TB", "PB", "EB" };
            if (byteCount == 0)
            {
                return $"0 {suf[0]}";
            }
            var bytes = Abs(byteCount);
            var place = ToInt32(Floor(Log(bytes, 1024)));
            var num = Round(bytes / Pow(1024, place), 1);
            return $"{(Sign(byteCount) * num)} {suf[place]}";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}