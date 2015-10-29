using System;
using System.Collections.Generic;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    sealed class TimerItem : SimpleItem<TimerEntry, Toggl.TogglAutocompleteView>
    {
        public static TimerItem ForDescriptionCompletion(Toggl.TogglAutocompleteView item)
        {
            return new TimerItem(item, item.Description + " " + item.ProjectLabel + " " + item.TaskLabel + " " + item.ClientLabel);
        }

        public TimerItem(Toggl.TogglAutocompleteView item, bool isProject)
            : this(item, isProject ? item.ProjectAndTaskLabel : item.Description)
        {
        }

        private TimerItem(Toggl.TogglAutocompleteView item, string text)
            : base(item, text)
        {

        }

        protected override TimerEntry createElement(Action selectWithClick, List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<TimerEntry>()
                .Initialised(this.Item, selectWithClick)
                .MarkForRecycling(recyclables);
        }
    }
}