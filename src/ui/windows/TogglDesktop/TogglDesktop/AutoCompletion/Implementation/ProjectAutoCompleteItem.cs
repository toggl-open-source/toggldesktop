using System;
using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ProjectAutoCompleteItem : SimpleAutoCompleteItem<ProjectAutoCompleteEntry, Toggl.AutocompleteItem>
    {
        public ProjectAutoCompleteItem(Toggl.AutocompleteItem item) : base(item, item.ProjectLabel)
        {
        }

        protected override ProjectAutoCompleteEntry createElement(Action selectWithClick)
        {
            return new ProjectAutoCompleteEntry(this.Item, selectWithClick);
        }
    }
}