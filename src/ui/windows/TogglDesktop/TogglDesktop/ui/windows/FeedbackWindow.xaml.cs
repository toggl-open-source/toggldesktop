using System.Windows;
using System.Windows.Controls;
using Microsoft.Win32;

namespace TogglDesktop
{
    public partial class FeedbackWindow
    {
        private string attachedFileName;

        public FeedbackWindow()
        {
            this.InitializeComponent();
            this.reset();
        }

        private void reset()
        {
            this.topicComboBox.SelectedItem = null;
            this.messageTextBox.Text = "";
            this.attachedFileText.Text = "";
            this.noTopicText.Visibility = Visibility.Hidden;
            this.topicEmptyText.Visibility = Visibility.Visible;
            this.attachedFileName = null;
        }

        protected override void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.Hide();
            this.reset();
        }

        private void onTopicSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            this.noTopicText.Visibility = Visibility.Hidden;
            this.topicEmptyText.Visibility = Visibility.Hidden;
        }

        private void onAttachImageClick(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            if ((bool)dialog.ShowDialog(this))
            {
                this.attachedFileName = dialog.FileName;
                this.attachedFileText.Text = "Attached: " + dialog.SafeFileName;
            }
        }

        private void onSendClick(object sender, RoutedEventArgs e)
        {
            if (this.topicComboBox.SelectedItem == null)
            {
                this.noTopicText.Visibility = Visibility.Visible;
                this.topicComboBox.Focus();
                return;
            }

            if (this.messageTextBox.Text == "")
            {
                this.messageTextBox.Focus();
                return;
            }

            if (!Toggl.SendFeedback(
                this.topicComboBox.Text,
                this.messageTextBox.Text,
                this.attachedFileName))
            {
                return;
            }

            MessageBox.Show(this, "Your feedback was sent successfully.", "Thank you!");
            
            this.reset();
            this.Hide();
        }

    }
}
