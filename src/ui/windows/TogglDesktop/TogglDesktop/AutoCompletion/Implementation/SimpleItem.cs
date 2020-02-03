namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class SimpleItem<TAutoCompleteItem> : AutoCompleteItem
    {
        protected SimpleItem(TAutoCompleteItem item, string text)
            : base(text)
        {
            this.Item = item;
        }

        public TAutoCompleteItem Item { get; }
    }
}