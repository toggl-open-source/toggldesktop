namespace TogglDesktop.AutoCompletion.Items
{
    class ProjectItem : TimeEntryItem
    {
        public ProjectItem(TimeEntryItem item)
            : base(CreateProjectItem(item.Model))
        {
        }

        private static Toggl.TogglAutocompleteView CreateProjectItem(Toggl.TogglAutocompleteView from)
        {
            var projectItemCopy = from;
            projectItemCopy.Description = string.Empty;
            projectItemCopy.TaskID = 0ul;
            projectItemCopy.TaskLabel = string.Empty;
            projectItemCopy.ProjectAndTaskLabel = projectItemCopy.ProjectLabel;
            projectItemCopy.Text = projectItemCopy.ProjectLabel;
            projectItemCopy.Type = 2;
            return projectItemCopy;
        }
    }
}