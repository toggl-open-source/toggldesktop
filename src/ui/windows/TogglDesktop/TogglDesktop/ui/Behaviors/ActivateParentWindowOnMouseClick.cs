using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Interactivity;

namespace TogglDesktop.Behaviors
{
    public class ActivateParentWindowOnMouseClick : Behavior<Popup>
    {
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.PreviewMouseDown += PopupOnPreviewMouseDown;
        }

        private static void PopupOnPreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            System.Windows.Application.Current?.MainWindow?.Activate();
        }

        protected override void OnDetaching()
        {
            AssociatedObject.PreviewMouseDown -= PopupOnPreviewMouseDown;
            base.OnDetaching();
        }
    }
}