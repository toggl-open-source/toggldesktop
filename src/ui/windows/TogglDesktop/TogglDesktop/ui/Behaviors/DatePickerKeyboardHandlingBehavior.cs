using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interactivity;

namespace TogglDesktop.Behaviors
{
    public class DatePickerKeyboardHandlingBehavior : Behavior<DatePicker>
    {
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.PreviewKeyDown += OnDatePickerPreviewKeyDown;
        }

        private void OnDatePickerPreviewKeyDown(object sender, KeyEventArgs args)
        {
            if (args.Key == Key.Enter)
            {
                if (AssociatedObject.IsDropDownOpen == false)
                {
                    args.Handled = true;
                    AssociatedObject.IsDropDownOpen = true;
                }
            }

            if (!AssociatedObject.SelectedDate.HasValue)
            {
                return;
            }

            var date = AssociatedObject.SelectedDate.Value;
            if (args.Key == Key.Up)
            {
                args.Handled = true;
                AssociatedObject.SelectedDate = date.AddDays(1);
            }
            else if (args.Key == Key.Down)
            {
                args.Handled = true;
                AssociatedObject.SelectedDate = date.AddDays(-1);
            }
        }

        protected override void OnDetaching()
        {
            AssociatedObject.PreviewKeyDown -= OnDatePickerPreviewKeyDown;
            base.OnDetaching();
        }
    }
}