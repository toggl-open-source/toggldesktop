using System.ComponentModel;
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
            this.Closing += ResetOnClosing;
            this.reset();
        }

        private void ResetOnClosing(object sender, CancelEventArgs e)
        {
            this.reset();
        }

        private void reset()
        {
            this.topicComboBox.SelectedItem = null;
            this.messageTextBox.Text = "";
            this.attachedFileText.Text = "(maximum image size 5MB)";
            this.errorText.Visibility = Visibility.Hidden;
            this.attachedFileName = null;
        }

        private void onTopicSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            this.errorText.Visibility = Visibility.Hidden;
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
                this.errorText.Text = "Please choose a topic!";
                this.errorText.Visibility = Visibility.Visible;

                this.topicComboBox.Focus();
                return;
            }

            if (this.messageTextBox.Text == "")
            {
                this.errorText.Text = "Please type in your feedback!";
                this.errorText.Visibility = Visibility.Visible;
                this.messageTextBox.Focus();
                return;
            }

            if (!Toggl.SendFeedback(
                this.topicComboBox.Text,
                this.messageTextBox.Text,
                this.attachedFileName))
            {
                this.errorText.Text = "File upload failed! (check file size)";
                this.errorText.Visibility = Visibility.Visible;
                return;
            }

            MessageBox.Show(this, "Your feedback was sent successfully.", "Thank you!");
            
            this.reset();
            this.Hide();
        }
    }
}
