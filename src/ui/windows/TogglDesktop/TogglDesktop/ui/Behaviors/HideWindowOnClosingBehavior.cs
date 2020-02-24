using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Threading;
using Microsoft.Xaml.Behaviors;

namespace TogglDesktop.Behaviors
{
    public class HideWindowOnClosingBehavior : Behavior<Window>
    {
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.Closing += HideWindowOnClosing;
        }

        protected override void OnDetaching()
        {
            AssociatedObject.Closing -= HideWindowOnClosing;
            base.OnDetaching();
        }

        private void HideWindowOnClosing(object sender, CancelEventArgs e)
        {
            e.Cancel = true;
            Action hideAction = AssociatedObject.Hide;
            AssociatedObject.Dispatcher.BeginInvoke(DispatcherPriority.Background, hideAction);
        }
    }
}