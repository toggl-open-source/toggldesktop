using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class FeedbackWindow
    {
        public FeedbackWindowViewModel ViewModel
        {
            get => (FeedbackWindowViewModel)DataContext;
            private set => DataContext = value;
        }

        public FeedbackWindow()
        {
            this.InitializeComponent();
            this.Closing += ResetOnClosing;
            this.ResetState();
        }

        private void ResetOnClosing(object sender, CancelEventArgs e)
        {
            this.ResetState();
        }

        private void ResetState()
        {
            ViewModel = new FeedbackWindowViewModel(RefreshValidationBindings, OnFeedbackSent);
        }

        private void RefreshValidationBindings()
        {
            RefreshTopicComboBoxBinding();
            RefreshFeedbackTextBoxBinding();
        }

        private void OnFeedbackSent()
        {
            MessageBox.Show(this, "Our support team will be notified.", "Feedback sent successfully!");
            this.Close();
        }

        private void OnTopicComboBoxLostFocus(object sender, RoutedEventArgs e)
        {
            if (!topicComboBox.IsKeyboardFocusWithin)
            {
                RefreshTopicComboBoxBinding();
            }
        }

        private void OnFeedbackTextBoxLostFocus(object sender, RoutedEventArgs e)
        {
            if (!feedbackTextBox.IsKeyboardFocusWithin && !topicComboBox.IsKeyboardFocusWithin)
            {
                RefreshFeedbackTextBoxBinding();
            }
        }

        private void RefreshFeedbackTextBoxBinding() => feedbackTextBox.GetBindingExpression(TextBox.TextProperty).UpdateSource();
        private void RefreshTopicComboBoxBinding() => topicComboBox.GetBindingExpression(ComboBox.SelectedItemProperty).UpdateSource();
    }
}
