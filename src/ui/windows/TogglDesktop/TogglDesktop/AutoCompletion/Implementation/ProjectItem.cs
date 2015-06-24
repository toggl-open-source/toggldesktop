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

        public override IEnumerable<AutoCompleteItem> Complete(string input)
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
        public ProjectItem(Toggl.AutocompleteItem item) : base(item, item.ProjectLabel)
        {
        }

        protected override ProjectEntry createElement(Action selectWithClick)
        {
            return new ProjectEntry(this.Item, selectWithClick);
        }
    }
}