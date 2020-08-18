using System.Windows;
using System.Windows.Input;

namespace TogglDesktop
{
    public class DatePickerHelper : DependencyObject
    {
        public static readonly DependencyProperty PreviousDayCommandProperty = 
            DependencyProperty.RegisterAttached("PreviousDayCommand", typeof(ICommand),
                typeof(DatePickerHelper));

        public static ICommand GetPreviousDayCommand(DatePickerHelper d)
        {
            return (ICommand) d.GetValue(PreviousDayCommandProperty);
        }

        public static void SetPreviousDayCommand(DatePickerHelper d, ICommand value)
        {
            d.SetValue(PreviousDayCommandProperty, value);
        }

        public static readonly DependencyProperty NextDayCommandProperty =
            DependencyProperty.RegisterAttached("NextDayCommand", typeof(ICommand),
                typeof(DatePickerHelper));

        public static ICommand GetNextDayCommand(DatePickerHelper d)
        {
            return (ICommand)d.GetValue(PreviousDayCommandProperty);
        }

        public static void SetNextDayCommand(DatePickerHelper d, ICommand value)
        {
            d.SetValue(PreviousDayCommandProperty, value);
        }
    }
}
