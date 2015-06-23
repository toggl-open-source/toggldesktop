using System;
using System.Collections.Generic;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class StringItem : SimpleItem<StringEntry, string>
    {
        private readonly Func<string, bool> hideString;

        public StringItem(string item, Func<string, bool> hideString)
            : base(item, item)
        {
            this.hideString = hideString;
        }

        public override IEnumerable<AutoCompleteItem> Complete(string input)
        {
            this.Visible = !this.hideString(this.Item) && this.Text.Contains(input);

            if (this.Visible)
            {
                yield return this;
            }
        }

        protected override StringEntry createElement(Action selectWithClick)
        {
            return new StringEntry(this.Item, selectWithClick);
        }
    }
}