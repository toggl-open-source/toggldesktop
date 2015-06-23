using System;
using System.Windows;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    sealed class StringEntrySample
    {
        public string Text { get { return "This is text"; } }

        public Color BackgroundColor { get { return Color.FromRgb(255, 255, 255); } }
    }

    partial class StringEntry
    {
        public StringEntry(string item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.Description = item;
            InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty TextProperty = DependencyProperty
            .Register("Text", typeof(string), typeof(StringEntry));

        public string Description
        {
            get { return (string)this.GetValue(TextProperty); }
            set { this.SetValue(TextProperty, value); }
        }

        #endregion

    }
}
