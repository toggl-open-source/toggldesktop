namespace TogglDesktop.AutoCompletion
{
    class ListBoxItemViewModel
    {
        public string Text { get; protected set; }
        public ItemType Type { get; protected set; }
        public int Index { get; protected set; }
    }

    class StringItemViewModel : ListBoxItemViewModel
    {
        public StringItemViewModel(string text, int index)
        {
            Text = text;
            Type = ItemType.STRINGITEM;
            Index = index;
        }
    }

    class TagItemViewModel : ListBoxItemViewModel
    {
        public bool IsChecked { get; set; }
        public TagItemViewModel(string text, int index)
        {
            Text = text;
            Type = ItemType.TAGITEM;
            Index = index;
        }
    }

    class WorkspaceSeparatorItemViewModel : ListBoxItemViewModel
    {
        public static WorkspaceSeparatorItemViewModel Instance => new WorkspaceSeparatorItemViewModel();
        private WorkspaceSeparatorItemViewModel()
        {
            Type = ItemType.WORKSPACE_SEPARATOR;
        }
    }

    class WorkspaceItemViewModel : ListBoxItemViewModel
    {
        public WorkspaceItemViewModel(string workspaceName)
        {
            Text = workspaceName;
            Type = ItemType.WORKSPACE;
        }
    }

    class CategoryItemViewModel : ListBoxItemViewModel
    {
        public string Category { get; }
        public CategoryItemViewModel(string category)
        {
            Category = category;
            Type = ItemType.CATEGORY;
        }
    }

    class ProjectItemViewModel : TimeEntryItemViewModel
    {
        public ProjectItemViewModel(TimeEntryItemViewModel item, int index)
        {
            Text = item.ProjectLabel;
            ProjectLabel = item.ProjectLabel;
            ProjectColor = item.ProjectColor;
            ClientLabel = item.ClientLabel;
            Type = ItemType.PROJECT;
            WorkspaceName = item.WorkspaceName;
            Index = index;
        }

        protected ProjectItemViewModel()
        { }
    }

    class TimeEntryItemViewModel : ListBoxItemViewModel
    {
        public string Description { get; }
        public string TaskLabel { get; }
        public string ProjectAndTaskLabel { get; }
        public string ProjectLabel { get; protected set; }
        public string ProjectColor { get; protected set; }
        public string ClientLabel { get; protected set; }
        public string WorkspaceName { get; protected set; }
        public TimeEntryItemViewModel(Toggl.TogglAutocompleteView item, int index)
        {
            var taskLabel = item.Type == 0
                ? ((item.TaskLabel.Length > 0) ? $" - {item.TaskLabel}" : string.Empty)
                : item.TaskLabel;
            var clientLabel = (item.ClientLabel.Length > 0) ? $" {item.ClientLabel}" : string.Empty;
            Text = item.Text;
            Description = item.Description;
            ProjectLabel = item.ProjectLabel;
            ProjectColor = item.ProjectColor;
            ProjectAndTaskLabel = item.ProjectAndTaskLabel;
            TaskLabel = taskLabel;
            ClientLabel = clientLabel;
            Type = (ItemType)((int)item.Type);
            WorkspaceName = item.WorkspaceName;
            Index = index;
        }

        protected TimeEntryItemViewModel()
        {
        }
    }

    class NoProjectItemViewModel : ProjectItemViewModel
    {
        public static NoProjectItemViewModel Instance => new NoProjectItemViewModel();
        private NoProjectItemViewModel()
        {
            Text = "No project";
            ProjectLabel = "No project";
            Type = ItemType.PROJECT;
            Index = -1;
        }
    }

    class ClientItemViewModel : ListBoxItemViewModel
    {
        public ClientItemViewModel(string clientLabel)
        {
            Text = string.IsNullOrEmpty(clientLabel) ? "No client" : clientLabel;
            Type = ItemType.CLIENT;
        }
    }

    class CustomTextItemViewModel : ListBoxItemViewModel
    {
        public string Title { get; }
        public CustomTextItemViewModel(string title, string text)
        {
            Title = title;
            Text = text;
            Type = ItemType.CUSTOM_TEXT;
        }
    }

    static class ListBoxItemViewModelExtensions
    {
        public static bool IsSelectable(this ListBoxItemViewModel item) => item.IsModelItem();
        public static bool IsModelItem(this ListBoxItemViewModel item) => (int) item.Type >= 0;
    }
}
