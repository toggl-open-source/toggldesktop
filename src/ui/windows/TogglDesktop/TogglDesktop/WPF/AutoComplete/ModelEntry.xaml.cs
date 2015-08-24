using System;

namespace TogglDesktop.WPF.AutoComplete
{
    partial class ModelEntry
    {
        public ModelEntry(Toggl.TogglGenericView item, Action selectWithClick, string overrideText = null)
            : base(selectWithClick)
        {
            this.InitializeComponent();
            this.text.Text = overrideText ?? item.Name;
        }
    }
}
