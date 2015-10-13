using System;
using System.Collections.Generic;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class StringItem : SimpleItem<StringEntry, string>
    {
        private readonly Func<string, bool> hideString;

        public StringItem(string item)
            : this(item, s => true)
        {
        }

        public StringItem(string item, Func<string, bool> hideString)
            : base(item, item)
        {
            this.hideString = hideString;
        }


        public override IEnumerable<AutoCompleteItem> Complete(string[] words)
        {
            this.Visible = !this.hideString(this.Item) && this.completesAll(words);

            if (this.Visible)
            {
                yield return this;
            }
        }

        protected override StringEntry createElement(Action selectWithClick, List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<StringEntry>()
                .Initialised(this.Item, selectWithClick)
                .MarkForRecycling(recyclables);
        }
    }
}