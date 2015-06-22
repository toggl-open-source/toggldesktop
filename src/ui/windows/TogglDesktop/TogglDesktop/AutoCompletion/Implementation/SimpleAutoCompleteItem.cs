using System;
using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    abstract class SimpleAutoCompleteItem<TFrameworkElement, TAutoCompleteItem> : AutoCompleteItem
        where TFrameworkElement : FrameworkElement, ISelectable
    {
        private readonly TAutoCompleteItem item;
        private TFrameworkElement element;

        protected SimpleAutoCompleteItem(TAutoCompleteItem item, string text)
            : base(text)
        {
            this.item = item;
        }

        public TAutoCompleteItem Item
        {
            get { return this.item; }
        }

        public override void CreateFrameworkElement(Panel parent, Action<AutoCompleteItem> selectWithClick)
        {
            this.element = this.createElement(() => selectWithClick(this));
            parent.Children.Add(this.element);
        }

        protected abstract TFrameworkElement createElement(Action selectWithClick);

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