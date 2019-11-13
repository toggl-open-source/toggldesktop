using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
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

        private void OnCancelClick(object sender, RoutedEventArgs e)
        {
            this.ResetState();
            this.Hide();
        }

        private void RefreshValidationBindings()
        {
            RefreshTopicComboBoxBinding();
            RefreshFeedbackTextBoxBinding();
        }

        private void OnFeedbackSent()
        {
            MessageBox.Show(this, "Our support team will be notified.", "Feedback sent successfully!");
            this.ResetState();
            this.Hide();
        }

        private void OnFeedbackTextBoxValidationError(object sender, ValidationErrorEventArgs e)
        {
            if (!Equals(Keyboard.FocusedElement, topicComboBox))
            {
                feedbackTextBox.Dispatcher.BeginInvoke(new Action(() => Keyboard.Focus(feedbackTextBox)), System.Windows.Threading.DispatcherPriority.Background);
            }
        }

        private void OnTopicComboBoxValidationError(object sender, ValidationErrorEventArgs e)
        {
            topicComboBox.Dispatcher.BeginInvoke(new Action(() => Keyboard.Focus(topicComboBox)), System.Windows.Threading.DispatcherPriority.Background);
        }

        private void OnTopicComboBoxLostFocus(object sender, RoutedEventArgs e) => RefreshTopicComboBoxBinding();
        private void OnFeedbackTextBoxLostFocus(object sender, RoutedEventArgs e) => RefreshFeedbackTextBoxBinding();
        private void RefreshFeedbackTextBoxBinding() => feedbackTextBox.GetBindingExpression(TextBox.TextProperty).UpdateSource();
        private void RefreshTopicComboBoxBinding() => topicComboBox.GetBindingExpression(ComboBox.SelectedItemProperty).UpdateSource();
    }
}
