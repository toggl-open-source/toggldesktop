using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class SimpleItem<TFrameworkElement, TAutoCompleteItem> : AutoCompleteItem
        where TFrameworkElement : FrameworkElement, ISelectable
    {
        private readonly TAutoCompleteItem item;
        private TFrameworkElement element;

        protected SimpleItem(TAutoCompleteItem item, string text)
            : base(text)
        {
            this.item = item;
        }

        public TAutoCompleteItem Item
        {
            get { return this.item; }
        }

        public override void CreateFrameworkElement(
            Panel parent, Action<AutoCompleteItem> selectWithClick,
            List<IRecyclable> recyclables, AutoCompleteController controller)
        {
            this.element = this.createElement(() => selectWithClick(this), recyclables);
            this.element.Visibility = Visibility.Visible;
            parent.Children.Add(this.element);
        }

        protected abstract TFrameworkElement createElement(Action selectWithClick, List<IRecyclable> recyclables);

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