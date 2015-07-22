using System.Collections.Generic;
using System.Windows;
using TogglDesktop.WPF.AutoComplete;

namespace TogglDesktop.AutoCompletion.Implementation
{
    sealed class SimpleNoncompletingCategory : SimpleCategory
    {
        private readonly string text;

        public SimpleNoncompletingCategory(string text, List<IAutoCompleteListItem> children)
            : base("", children)
        {
            this.text = text;
        }

        protected override UIElement createElement()
        {
            return new GrayTextCategory(this.text);
        }
    }
}