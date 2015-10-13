
using System.Windows;

namespace TogglDesktop
{
    public partial class ErrorBar
    {
        public ErrorBar()
        {
            this.InitializeComponent();
        }

        public void ShowError(string errorText)
        {
            this.errorText.Text = errorText;
            this.errorText.ToolTip = errorText;
            this.Visibility = Visibility.Visible;
        }

        public void Hide()
        {
            this.Visibility = Visibility.Collapsed;
        }

        private void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.Hide();
        }
    }
}
