using System.Windows;
using System.Windows.Input;
using ControlzEx;

namespace TogglDesktop
{
    public partial class MessageBox
    {
        private MessageBoxResult result;

        private MessageBox()
        {
            this.InitializeComponent();
        }

        public static MessageBoxResult Show(Window owner, string messageText,
            string title = "Toggl Desktop", MessageBoxButton buttons = MessageBoxButton.OK,
            string okButtonText = "OK")
        {
            var box = StaticObjectPool.PopOrDefault<MessageBox>()
                ?? new MessageBox();

            var result = box.showDialog(owner, messageText, title, buttons, okButtonText);

            StaticObjectPool.Push(box);

            return result;
        }

        private MessageBoxResult showDialog(Window owner, string messageText,
            string title, MessageBoxButton buttons, string okButtonText)
        {
            this.setup(owner, messageText, title, buttons, okButtonText);

            this.result = MessageBoxResult.None;
            this.ShowDialog();

            this.Owner = null;

            return this.result;
        }

        private void setup(Window owner, string messageText,
            string title, MessageBoxButton buttons, string okButtonText)
        {
            this.Owner = owner;
            this.messageTextBlock.Text = messageText;
            this.titleTextBlock.Text = title;

            var ok = buttons == MessageBoxButton.OK || buttons == MessageBoxButton.OKCancel;
            var yesno = buttons == MessageBoxButton.YesNo || buttons == MessageBoxButton.YesNoCancel;
            var cancel = buttons == MessageBoxButton.OKCancel || buttons == MessageBoxButton.YesNoCancel;

            this.okButton.ShowOnlyIf(ok);
            this.yesButton.ShowOnlyIf(yesno);
            this.noButton.ShowOnlyIf(yesno);
            this.cancelButton.ShowOnlyIf(cancel);

            this.okButton.Content = okButtonText;

            KeyboardNavigationEx.Focus(ok ? okButton : yesButton);

            if (owner != null)
            {
                this.Topmost = owner.Topmost;
            }
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            var key = e.Key;

            if (key == Key.Escape && this.cancelButton.IsVisible)
            {
                e.Handled = true;
                this.close(MessageBoxResult.Cancel);
            }
            else if (key == Key.Enter && this.okButton.IsVisible)
            {
                e.Handled = true;
                this.close(MessageBoxResult.OK);
            }
        }

        private void close(MessageBoxResult result)
        {
            this.result = result;
            this.Close();
        }

        private void onOkButtonClick(object sender, RoutedEventArgs e)
        {
            this.close(MessageBoxResult.OK);
        }

        private void onYesButtonClick(object sender, RoutedEventArgs e)
        {
            this.close(MessageBoxResult.Yes);
        }
        private void onNoButtonClick(object sender, RoutedEventArgs e)
        {
            this.close(MessageBoxResult.No);
        }
        private void onCancelButtonClick(object sender, RoutedEventArgs e)
        {
            this.close(MessageBoxResult.Cancel);
        }
    }
}
