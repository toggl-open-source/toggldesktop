using System;
using TogglDesktop.WPF;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ProjectAutoCompleteItem : SimpleAutoCompleteItem<ProjectEntry, Toggl.AutocompleteItem>
    {
        public ProjectAutoCompleteItem(Toggl.AutocompleteItem item) : base(item, item.ProjectLabel)
        {
        }

        protected override ProjectEntry createElement(Action selectWithClick)
        {
            return new ProjectEntry(this.Item, selectWithClick);
        }
    }
}