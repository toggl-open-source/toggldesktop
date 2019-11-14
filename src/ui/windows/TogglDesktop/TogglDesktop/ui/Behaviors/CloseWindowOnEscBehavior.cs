using System.Windows;
using System.Windows.Input;
using System.Windows.Interactivity;

namespace TogglDesktop.Behaviors
{
    public class CloseWindowOnEscBehavior : Behavior<Window>
    {
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.KeyDown += AssociatedObjectOnKeyDown;
        }

        private void AssociatedObjectOnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                AssociatedObject.Close();
                e.Handled = true;
            }
        }

        protected override void OnDetaching()
        {
            AssociatedObject.KeyDown -= AssociatedObjectOnKeyDown;
            base.OnDetaching();
        }
    }
}