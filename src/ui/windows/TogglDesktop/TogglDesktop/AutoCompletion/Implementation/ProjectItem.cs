using System;
using System.Collections.Generic;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    interface IProjectItem
    {
        Toggl.TogglAutocompleteView Item { get; }
    }

    class NoProjectItem : ProjectItem
    {
        public static IAutoCompleteListItem Create()
        {
            return new NoProjectItem();
        }

        private NoProjectItem()
            : base(new Toggl.TogglAutocompleteView())
        {
        }

        public override IEnumerable<AutoCompleteItem> Complete(string[] input)
        {
            yield return this;
        }

        protected override ProjectEntry createElement(Action selectWithClick, List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<ProjectEntry>()
                .Initialised(this.Item, selectWithClick, "No project")
                .MarkForRecycling(recyclables);
        }
    }

    class ProjectItem : SimpleItem<ProjectEntry, Toggl.TogglAutocompleteView>, IProjectItem
    {
        public static ProjectItem Create(Toggl.TogglAutocompleteView item)
        {
            return new ProjectItem(item);
        }

        protected ProjectItem(Toggl.TogglAutocompleteView item)
            : base(item, createAutocompletionString(item))
        {
        }

        private static string createAutocompletionString(Toggl.TogglAutocompleteView item)
        {
            if (string.IsNullOrEmpty(item.ClientLabel))
            {
                if (string.IsNullOrEmpty(item.TaskLabel))
                {
                    return item.ProjectLabel;
                }
                else
                {
                    return item.ProjectLabel + " " + item.TaskLabel;
                }
            }
            else
            {
                if (string.IsNullOrEmpty(item.TaskLabel))
                {
                    return item.ProjectLabel + " " + item.ClientLabel;
                }
                else
                {
                    return item.ProjectLabel + " " + item.ClientLabel + " " + item.TaskLabel;
                }
            }
        }

        protected override ProjectEntry createElement(Action selectWithClick, List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<ProjectEntry>()
                .Initialised(this.Item, selectWithClick)
                .MarkForRecycling(recyclables);
        }
    }
}