using System;
using System.Windows;
using System.Windows.Media;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    sealed class StringAutoCompleteEntrySample
    {
        public string Text { get { return "This is text"; } }

        public Color BackgroundColor { get { return Color.FromRgb(255, 255, 255); } }
    }

    partial class StringAutoCompleteEntry
    {
        public StringAutoCompleteEntry(string item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.Description = item;
            InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty TextProperty = DependencyProperty
            .Register("Text", typeof(string), typeof(StringAutoCompleteEntry));

        public string Description
        {
            get { return (string)this.GetValue(TextProperty); }
            set { this.SetValue(TextProperty, value); }
        }

        #endregion

    }
}
