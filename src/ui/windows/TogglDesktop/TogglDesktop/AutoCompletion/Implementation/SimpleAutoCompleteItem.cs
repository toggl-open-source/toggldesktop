using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class SimpleAutoCompleteItem<TFrameworkElement> : AutoCompleteItem
        where TFrameworkElement : FrameworkElement, ISelectable
    {
        private readonly Toggl.AutocompleteItem item;
        private TFrameworkElement element;

        protected SimpleAutoCompleteItem(Toggl.AutocompleteItem item, string text)
            : base(text)
        {
            this.item = item;
        }

        public Toggl.AutocompleteItem Item
        {
            get { return this.item; }
        }

        public override void CreateFrameworkElement(Panel parent)
        {
            this.element = this.createElement();
            parent.Children.Add(this.element);
        }

        protected abstract TFrameworkElement createElement();

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