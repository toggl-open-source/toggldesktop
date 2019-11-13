using System;
using System.Globalization;
using System.IO;
using System.Windows.Data;

namespace TogglDesktop.Converters
{
    public class TrimPathConverter : IValueConverter
    {
        public int TrimThreshold { get; set; } = 34;
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var path = value as string;
            if (path == null) return null;
            var fileName = Path.GetFileName(path);
            if (fileName.Length <= TrimThreshold) return fileName;
            return fileName.Substring(0, TrimThreshold / 2) + "..." +
                   fileName.Substring(fileName.Length - TrimThreshold / 2);

        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}