namespace TogglDesktop.AutoCompletion.Items
{
    class TimeEntryItem : AutoCompleteItem, IModelItem<Toggl.TogglAutocompleteView>
    {
        public string Description => Model.Description;
        public string TaskLabel => Model.TaskLabel;
        public string ProjectLabel => Model.ProjectLabel;
        public string ProjectColor => Model.ProjectColor;
        public string ClientLabel => Model.ClientLabel;
        public string WorkspaceName => Model.WorkspaceName;
        public Toggl.TogglAutocompleteView Model { get; }
        public TimeEntryItem(Toggl.TogglAutocompleteView item)
            : base(item.Type == 1 ? item.GetFullProjectString() : item.Text, (ItemType)(int)item.Type)
        {
            Model = item;
        }
    }
}