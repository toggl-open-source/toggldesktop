using System.ComponentModel;
using System.Windows;
using Microsoft.Xaml.Behaviors;

namespace TogglDesktop.Behaviors
{
    public class FocusParentWindowOnClosingBehavior : Behavior<Window>
    {
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.Closing += OnClosing;
        }

        protected override void OnDetaching()
        {
            AssociatedObject.Closing -= OnClosing;
            base.OnDetaching();
        }

        private void OnClosing(object sender, CancelEventArgs e)
        {
            AssociatedObject.Owner?.Focus();
        }
    }
}