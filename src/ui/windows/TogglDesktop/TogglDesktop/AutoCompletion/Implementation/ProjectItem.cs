using System;
using System.Collections.Generic;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class NoProjectItem : ProjectItem
    {
        public NoProjectItem()
            : base(new Toggl.AutocompleteItem())
        {
        }

        public override IEnumerable<AutoCompleteItem> Complete(string[] input)
        {
            yield return this;
        }

        protected override ProjectEntry createElement(Action selectWithClick)
        {
            return new ProjectEntry(this.Item, selectWithClick, "No project");
        }
    }

    class ProjectItem : SimpleItem<ProjectEntry, Toggl.AutocompleteItem>
    {
        public ProjectItem(Toggl.AutocompleteItem item)
            : base(item, createAutocompletionString(item))
        {
        }

        private static string createAutocompletionString(Toggl.AutocompleteItem item)
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

        protected override ProjectEntry createElement(Action selectWithClick)
        {
            return new ProjectEntry(this.Item, selectWithClick);
        }
    }
}