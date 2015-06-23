using System;
using TogglDesktop.WPF;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ProjectItem : SimpleItem<ProjectEntry, Toggl.AutocompleteItem>
    {
        public ProjectItem(Toggl.AutocompleteItem item) : base(item, item.ProjectLabel)
        {
        }

        protected override ProjectEntry createElement(Action selectWithClick)
        {
            return new ProjectEntry(this.Item, selectWithClick);
        }
    }
}