namespace TogglDesktop.AutoCompletion.Items
{
    internal class GenericModelItem : ModelItem<Toggl.TogglGenericView>
    {
        public GenericModelItem(Toggl.TogglGenericView model)
            : base(model, model.Name, ItemType.STRINGITEM)
        {
        }
    }
}