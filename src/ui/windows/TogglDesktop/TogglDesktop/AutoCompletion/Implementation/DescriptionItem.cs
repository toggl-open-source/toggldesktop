using System;
using System.Collections.Generic;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class DescriptionItem : SimpleItem<DescriptionAutoCompleteEntry, Toggl.TogglAutocompleteView>
    {
        public DescriptionItem(Toggl.TogglAutocompleteView item) : base(item, item.Description)
        {
        }

        protected override DescriptionAutoCompleteEntry createElement(Action selectWithClick, List<IRecyclable> recyclables)
        {
            return new DescriptionAutoCompleteEntry(this.Item, selectWithClick);
        }
    }
}