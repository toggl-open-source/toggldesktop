namespace TogglDesktop.AutoCompletion.Items
{
    class NoProjectItem : ModelItem<Toggl.TogglAutocompleteView>
    {
        public static NoProjectItem Instance => new NoProjectItem();
        public string ProjectLabel { get; }
        private NoProjectItem()
            : base(new Toggl.TogglAutocompleteView(), "No project", ItemType.PROJECT)
        {
            ProjectLabel = "No project";
        }
    }
}