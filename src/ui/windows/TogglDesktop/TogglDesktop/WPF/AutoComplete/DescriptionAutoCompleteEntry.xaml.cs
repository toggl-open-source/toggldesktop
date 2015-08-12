using System;
using System.Windows;
using System.Windows.Media;

namespace TogglDesktop.WPF.AutoComplete
{
    sealed class DescriptionEntrySample
    {
        public string Description { get { return "Description? Hello. Yes, this is test"; } }

        public Color BackgroundColor { get { return Color.FromRgb(244, 244, 244); } }
    }

    partial class DescriptionAutoCompleteEntry
    {
        public DescriptionAutoCompleteEntry(Toggl.TogglAutocompleteView item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.Description = item.Description;
            this.InitializeComponent();
        }

        #region dependency properties

        public static readonly DependencyProperty DescriptionProperty = DependencyProperty
            .Register("Description", typeof(string), typeof(DescriptionAutoCompleteEntry));

        public string Description
        {
            get { return (string)this.GetValue(DescriptionProperty); }
            set { this.SetValue(DescriptionProperty, value); }
        }

        #endregion
    }
}
