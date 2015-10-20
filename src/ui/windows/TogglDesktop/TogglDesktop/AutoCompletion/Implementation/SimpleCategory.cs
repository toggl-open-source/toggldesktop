using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class SimpleCategory : AutoCompleteListCategory
    {
        private UIElement element;

        protected SimpleCategory(string text, List<IAutoCompleteListItem> children)
            : base(text, children)
        {
        }

        protected override Panel createFrameworkElement(Panel parent, List<IRecyclable> recyclables)
        {
            this.element = this.createElement(recyclables);
            this.element.Visibility = Visibility.Visible;
            parent.Children.Add(this.element);
            return parent;
        }

        protected abstract UIElement createElement(List<IRecyclable> recyclables);

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