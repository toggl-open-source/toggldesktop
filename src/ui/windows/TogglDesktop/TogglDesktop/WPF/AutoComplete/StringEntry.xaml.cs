using System;

namespace TogglDesktop.WPF.AutoComplete
{
    partial class StringEntry
    {
        public StringEntry(string item, Action selectWithClick)
            : base(selectWithClick)
        {
            this.text.Text = item;
            this.InitializeComponent();
        }
    }
}
