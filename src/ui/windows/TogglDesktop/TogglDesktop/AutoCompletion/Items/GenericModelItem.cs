namespace TogglDesktop.AutoCompletion.Items
{
    class GenericModelItem : ModelItem<Toggl.TogglGenericView>
    {
        public GenericModelItem(Toggl.TogglGenericView model)
            : base(model, model.Name, ItemType.STRINGITEM)
        {
        }
    }
}