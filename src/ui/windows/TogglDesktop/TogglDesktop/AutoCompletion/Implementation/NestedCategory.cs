using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class NestedCategory : AutoCompleteListCategory
    {
        private UIElement element;

        protected NestedCategory(string text, List<IAutoCompleteListItem> children)
            : base(text, children)
        {
        }

        protected override Panel createFrameworkElement(Panel parent, List<IRecyclable> recyclables)
        {
            Panel newPanel;
            this.element = this.createElement(out newPanel, recyclables);
            this.element.Visibility = Visibility.Visible;
            parent.Children.Add(this.element);
            return newPanel;
        }

        protected abstract UIElement createElement(out Panel childrenPanel, List<IRecyclable> recyclables);

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