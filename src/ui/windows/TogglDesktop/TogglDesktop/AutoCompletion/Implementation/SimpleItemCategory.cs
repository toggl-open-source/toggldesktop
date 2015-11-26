using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class SimpleItemCategory<TFrameworkElement, TAutoCompleteItem> : AutoCompleteItemCategory
        where TFrameworkElement : FrameworkElement, ISelectable
    {
        private readonly TAutoCompleteItem item;
        private TFrameworkElement element;

        protected SimpleItemCategory(TAutoCompleteItem item, string text, List<IAutoCompleteListItem> children)
            : base(text, children)
        {
            this.item = item;
        }

        public TAutoCompleteItem Item
        {
            get { return this.item; }
        }

        protected override Panel createFrameworkElement(
            Panel parent, Action<AutoCompleteItem> selectWithClick,
            List<IRecyclable> recyclables, out ICollapsable collapsable)
        {
            Panel newParent;
            this.element = this.createElement(() => selectWithClick(this), recyclables,
                out newParent, out collapsable);
            this.element.Visibility = Visibility.Visible;
            parent.Children.Add(this.element);
            return newParent;
        }

        protected abstract TFrameworkElement createElement(
            Action selectWithClick, List<IRecyclable> recyclables,
            out Panel newParent, out ICollapsable collapsable);

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