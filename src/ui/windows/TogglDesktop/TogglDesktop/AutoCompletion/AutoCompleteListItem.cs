namespace TogglDesktop.AutoCompletion
{
    abstract class AutoCompleteListItem : IAutoCompleteListItem
    {
        public string Text { get; }

        protected AutoCompleteListItem(string text)
        {
            this.Text = text;
        }
    }
}