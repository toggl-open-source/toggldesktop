using System;
using System.Collections.Generic;
using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class StringAutoCompleteItem : SimpleAutoCompleteItem<StringAutoCompleteEntry, string>
    {
        private readonly Func<string, bool> hideString;

        public StringAutoCompleteItem(string item, Func<string, bool> hideString)
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

        protected override StringAutoCompleteEntry createElement()
        {
            return new StringAutoCompleteEntry(this.Item);
        }
    }
}