namespace TogglDesktop.AutoCompletion.Items
{
    class WorkspaceSeparatorItem : AutoCompleteItem
    {
        public static WorkspaceSeparatorItem Instance => new WorkspaceSeparatorItem();
        private WorkspaceSeparatorItem()
            : base(null, ItemType.WORKSPACE_SEPARATOR)
        {
        }
    }
}