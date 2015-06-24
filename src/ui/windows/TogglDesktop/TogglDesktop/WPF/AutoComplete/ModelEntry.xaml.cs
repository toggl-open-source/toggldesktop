using System;
using System.Windows;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    sealed class ModelEntrySample
    {
        public string ModelName { get { return "Big money company"; } }

        public Color BackgroundColor { get { return Color.FromRgb(255, 255, 255); } }
    }

    partial class ModelEntry
    {
        public ModelEntry(Toggl.Model item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.ModelName = item.Name;
            InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty ModelNameProperty = DependencyProperty
            .Register("ModelName", typeof(string), typeof(ModelEntry));

        public string ModelName
        {
            get { return (string)this.GetValue(ModelNameProperty); }
            set { this.SetValue(ModelNameProperty, value); }
        }
        #endregion
    }
}
