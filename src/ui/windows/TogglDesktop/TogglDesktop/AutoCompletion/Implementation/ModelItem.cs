namespace TogglDesktop.AutoCompletion.Implementation
{
    class ModelItem : SimpleItem<Toggl.TogglGenericView>
    {
        public ModelItem(Toggl.TogglGenericView model)
            : base(model, model.Name)
        {
        }
    }
}