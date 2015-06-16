using System.Windows;
using System.Windows.Controls;
using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class DescriptionAutoCompleteItem : AutoCompleteItem
    {
        private readonly Toggl.AutocompleteItem item;
        private DescriptionAutoCompleteEntry element;

        public DescriptionAutoCompleteItem(Toggl.AutocompleteItem item)
            : base(item.Description)
        {
            this.item = item;
        }

        public override void CreateFrameworkElement(Panel parent)
        {
            this.element = new DescriptionAutoCompleteEntry(this.item);
            parent.Children.Add(this.element);
        }

        protected override void hide()
        {
            this.element.Visibility = Visibility.Collapsed;
        }

        protected override void show()
        {
            this.element.Visibility = Visibility.Visible;
        }

        protected override void select()
        {
            this.element.Selected = true;
            this.element.BringIntoView();
        }

        protected override void unselect()
        {
            this.element.Selected = false;
        }
    }
}