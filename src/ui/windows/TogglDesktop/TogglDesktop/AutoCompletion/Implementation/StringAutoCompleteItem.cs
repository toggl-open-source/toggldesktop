using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class StringAutoCompleteItem : SimpleAutoCompleteItem<StringAutoCompleteEntry, string>
    {
        public StringAutoCompleteItem(string item)
            : base(item, item)
        {
        }

        protected override StringAutoCompleteEntry createElement()
        {
            return new StringAutoCompleteEntry(this.Item);
        }
    }
}