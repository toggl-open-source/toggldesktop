using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.AutoCompletion
{
    public interface ISelectable
    {
        bool IsSelectable { get; }
    }

    public interface IModelItemViewModel
    {
        AutoCompleteItem Model { get; }
    }
    class ListBoxItemViewModel : ISelectable
    {
        public string Text { get; protected set; }
        public ItemType Type { get; protected set; }
        public bool IsSelectable => this.IsModelItem();
    }

    class ModelItemViewModel : ListBoxItemViewModel, IModelItemViewModel
    {
        public AutoCompleteItem Model { get; protected set; }
    }

    class StringItemViewModel : ModelItemViewModel
    {
        public StringItemViewModel(ModelItem modelItem)
            : this(modelItem, modelItem.Item.Name)
        {
        }
        public StringItemViewModel(StringItem stringItem)
            : this(stringItem, stringItem.Text)
        {
        }

        private StringItemViewModel(AutoCompleteItem model, string text)
        {
            Model = model;
            Text = text;
            Type = ItemType.STRINGITEM;
        }
    }

    class TagItemViewModel : ModelItemViewModel
    {
        public bool IsChecked { get; set; }
        public TagItemViewModel(StringItem stringItem)
        {
            Model = stringItem;
            Text = stringItem.Item;
            Type = ItemType.TAGITEM;
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
        public ProjectItemViewModel(TimeEntryItemViewModel item)
        {
            TimerItem = CreateProjectItem(item.TimerItem);
            Text = item.ProjectLabel;
            ProjectLabel = item.ProjectLabel;
            ProjectColor = item.ProjectColor;
            ClientLabel = item.ClientLabel;
            Type = ItemType.PROJECT;
            WorkspaceName = item.WorkspaceName;
        }

        private static TimerItem CreateProjectItem(TimerItem from)
        {
            var projectItemCopy = from.Item;
            projectItemCopy.Description = string.Empty;
            projectItemCopy.TaskID = 0ul;
            projectItemCopy.TaskLabel = string.Empty;
            projectItemCopy.ProjectAndTaskLabel = projectItemCopy.ProjectLabel;
            projectItemCopy.Text = projectItemCopy.ProjectLabel;
            projectItemCopy.Type = 2;
            return new TimerItem(projectItemCopy, true);
        }

        protected ProjectItemViewModel()
        { }
    }

    class TimeEntryItemViewModel : ListBoxItemViewModel, IModelItemViewModel
    {
        public string Description { get; }
        public string TaskLabel { get; }
        public string ProjectAndTaskLabel { get; }
        public string ProjectLabel { get; protected set; }
        public string ProjectColor { get; protected set; }
        public string ClientLabel { get; protected set; }
        public string WorkspaceName { get; protected set; }
        public TimerItem TimerItem { get; protected set; }
        public AutoCompleteItem Model => TimerItem;
        public TimeEntryItemViewModel(TimerItem timerItem)
        {
            TimerItem = timerItem;
            var item = timerItem.Item;
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
            TimerItem = new TimerItem(new Toggl.TogglAutocompleteView(), true);
            Text = "No project";
            ProjectLabel = "No project";
            Type = ItemType.PROJECT;
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
        public static bool IsModelItem(this ListBoxItemViewModel item) => (int) item.Type >= 0;
    }
}
