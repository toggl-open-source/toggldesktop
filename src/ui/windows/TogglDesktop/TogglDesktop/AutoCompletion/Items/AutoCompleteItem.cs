namespace TogglDesktop.AutoCompletion.Items
{
    internal class AutoCompleteItem : IAutoCompleteItem
    {
        public AutoCompleteItem(string text, ItemType type)
        {
            Text = text;
            Type = type;
        }

        public string Text { get; }
        public ItemType Type { get; }
    }
}
