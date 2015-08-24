using System;

namespace TogglDesktop.WPF.AutoComplete
{
    partial class DescriptionAutoCompleteEntry
    {
        public DescriptionAutoCompleteEntry(Toggl.TogglAutocompleteView item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.text.Text = item.Description;
            this.InitializeComponent();
        }
    }
}
