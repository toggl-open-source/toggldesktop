using System;
using System.Collections.Generic;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    sealed class TimerItem : SimpleItem<TimerEntry, Toggl.TogglAutocompleteView>
    {
        public TimerItem(Toggl.TogglAutocompleteView item, bool isProject)
            : base(item, isProject ? item.ProjectAndTaskLabel : item.Description)
        {
        }

        protected override TimerEntry createElement(Action selectWithClick, List<IRecyclable> recyclables)
        {
            return new TimerEntry(this.Item, selectWithClick);
        }
    }
}