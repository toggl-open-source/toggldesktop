using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    sealed class ClientCategory : NestedCategory
    {
        public ClientCategory(string text, List<IAutoCompleteListItem> children)
            : base(text, children)
        {
        }

        protected override UIElement createElement(out Panel childrenPanel)
        {
            var e = new WPF.AutoComplete.ClientCategory(this.Text);
            childrenPanel = e.ProjectPanel;
            return e;
        }
    }
}