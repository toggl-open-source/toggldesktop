using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class DescriptionAutoCompleteItem : SimpleAutoCompleteItem<DescriptionAutoCompleteEntry, Toggl.AutocompleteItem>
    {
        public DescriptionAutoCompleteItem(Toggl.AutocompleteItem item) : base(item, item.Description)
        {
        }

        protected override DescriptionAutoCompleteEntry createElement()
        {
            return new DescriptionAutoCompleteEntry(this.Item);
        }
    }
}