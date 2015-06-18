using System;
using System.Windows;

namespace TogglDesktop.WPF
{
    sealed class TagSample
    {
        public string Text { get { return "This is tag"; } }
        public bool UserHover { get { return false; } }
    }

    public partial class Tag
    {
        public event EventHandler RemoveClicked;

        public Tag(string text)
            : this()
        {
            this.Text = text;
        }

        public Tag()
        {
            this.DataContext = this;
            InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty TextProperty = DependencyProperty
            .Register("Text", typeof(string), typeof(Tag));

        public string Text
        {
            get { return (string)this.GetValue(TextProperty); }
            set { this.SetValue(TextProperty, value); }
        }

        public static readonly DependencyProperty UserHoverProperty = DependencyProperty
            .Register("UserHover", typeof(string), typeof(Tag));

        public string UserHover
        {
            get { return (string)this.GetValue(UserHoverProperty); }
            set { this.SetValue(UserHoverProperty, value); }
        }

        #endregion

        private void remove_OnClick(object sender, RoutedEventArgs e)
        {
            if (this.RemoveClicked != null)
                this.RemoveClicked(this, EventArgs.Empty);
        }
    }
}
