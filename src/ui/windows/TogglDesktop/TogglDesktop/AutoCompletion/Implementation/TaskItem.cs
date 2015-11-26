using System;
using System.Collections.Generic;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class TaskItem : SimpleItem<TaskEntry, Toggl.TogglAutocompleteView>, IProjectItem
    {
        public static TaskItem Create(Toggl.TogglAutocompleteView item)
        {
            return new TaskItem(item);
        }

        protected TaskItem(Toggl.TogglAutocompleteView item)
            : base(item, item.TaskLabel)
        {
        }

        protected override TaskEntry createElement(Action selectWithClick, List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<TaskEntry>()
                .Initialised(this.Item, selectWithClick)
                .MarkForRecycling(recyclables);
        }
    }
}