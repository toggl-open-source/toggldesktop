using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ProjectAutoCompleteItem : SimpleAutoCompleteItem<ProjectAutoCompleteEntry, Toggl.AutocompleteItem>
    {
        public ProjectAutoCompleteItem(Toggl.AutocompleteItem item) : base(item, item.ProjectLabel)
        {
        }

        protected override ProjectAutoCompleteEntry createElement()
        {
            return new ProjectAutoCompleteEntry(this.Item);
        }
    }
}