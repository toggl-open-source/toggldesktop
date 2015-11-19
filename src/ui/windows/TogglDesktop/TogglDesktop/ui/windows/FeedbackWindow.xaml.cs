using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Microsoft.Win32;

namespace TogglDesktop
{
    public class FeedbackWindowSample
    {
        public List<FeedbackWindow.ArticleContainer> Articles
        {
            get
            {
                return new[]
                {
                    new Toggl.TogglHelpArticleView
                    {
                        Name = "This is an article"
                    },
                    new Toggl.TogglHelpArticleView
                    {
                        Name = "This is another article with a long title"
                    },
                    new Toggl.TogglHelpArticleView
                    {
                        Name = "This is another article with a long title that is so long it is bound to be too long for one line"
                    },
                }.Select(v => new FeedbackWindow.ArticleContainer(v)).ToList();
            }
        }
    }

    public partial class FeedbackWindow
    {
        private readonly DispatcherTimer suggestArticleTimer;

        private string attachedFileName;

        public FeedbackWindow()
        {
            this.InitializeComponent();

            this.suggestArticleTimer = this.initSuggestionsTimer();

            this.reset();

            Toggl.OnDisplayHelpArticles += this.onDisplayHelpArticles;
        }

        private DispatcherTimer initSuggestionsTimer()
        {
            return new DispatcherTimer(
                TimeSpan.FromSeconds(1),
                DispatcherPriority.Background,
                this.suggestArticles,
                this.Dispatcher)
            {
                IsEnabled = false,
            };
        }

        private void reset()
        {
            this.articlesList.ItemsSource = null;
            this.topicComboBox.SelectedItem = null;
            this.messageTextBox.Text = "";
            this.attachedFileText.Text = "";
            this.noTopicText.Visibility = Visibility.Hidden;
            this.topicEmptyText.Visibility = Visibility.Visible;
            this.attachedFileName = null;
        }

        private void onDisplayHelpArticles(List<Toggl.TogglHelpArticleView> articles)
        {
            if (this.TryBeginInvoke(this.onDisplayHelpArticles, articles))
                return;

            this.articlesList.ItemsSource = articles.Select(v => new ArticleContainer(v)).ToList();
        }

        public class ArticleContainer
        {
            private readonly Toggl.TogglHelpArticleView view;

            public ArticleContainer(Toggl.TogglHelpArticleView view)
            {
                this.view = view;
            }

            public string Category { get { return this.view.Category; } }
            public string Name { get { return this.view.Name; } }
            public string URL { get { return this.view.URL; } }
        }

        private void suggestArticles(object sender, EventArgs e)
        {
            this.suggestArticleTimer.IsEnabled = false;
            Toggl.SearchHelpArticles(this.messageTextBox.Text);
        }

        private void onMessageTextBoxTextChanged(object sender, TextChangedEventArgs e)
        {
            if (this.suggestArticleTimer != null)
            {
                this.suggestArticleTimer.IsEnabled = !string.IsNullOrWhiteSpace(this.messageTextBox.Text);
            }
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


        private void helpArticleLinkClicked(object sender, RoutedEventArgs e)
        {
            var frameworkElement = sender as FrameworkElement;
            if (frameworkElement == null)
                return;

            var article = frameworkElement.DataContext as ArticleContainer;
            if (article == null)
                return;

            Process.Start("http://support.toggl.com/" + article.URL);
        }
    }
}
