namespace TogglTrack.AutoCompletion.Items
{
    abstract class ModelItem<T> : AutoCompleteItem, IModelItem<T>
    {
        protected ModelItem(T model, string text, ItemType type)
            : base(text, type)
        {
            Model = model;
        }

        public T Model { get; }
    }
}