namespace TogglDesktop.AutoCompletion.Items
{
    public interface IModelItem<out T> : IAutoCompleteItem
    {
        T Model { get; }
    }
}