using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class SimpleCategory : AutoCompleteListCategory
    {
        private UIElement element;

        protected SimpleCategory(string text, List<IAutoCompleteListItem> children)
            : base(text, children)
        {
        }

        protected override Panel createFrameworkElement(Panel parent)
        {
            this.element = this.createElement();
            parent.Children.Add(this.element);
            return parent;
        }

        protected abstract UIElement createElement();

        protected override void hide()
        {
            this.element.Visibility = Visibility.Collapsed;
        }

        protected override void show()
        {
            this.element.Visibility = Visibility.Visible;
        }

    }
}