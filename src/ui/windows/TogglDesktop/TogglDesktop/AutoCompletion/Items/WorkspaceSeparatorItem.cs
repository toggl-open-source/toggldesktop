namespace TogglDesktop.AutoCompletion.Items
{
    internal class WorkspaceSeparatorItem : AutoCompleteItem
    {
        public static WorkspaceSeparatorItem Instance => new WorkspaceSeparatorItem();
        private WorkspaceSeparatorItem()
            : base(null, ItemType.WORKSPACE_SEPARATOR)
        {
        }
    }
}
