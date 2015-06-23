using System.Collections.Generic;
using System.Windows;

namespace TogglDesktop.AutoCompletion.Implementation
{
    sealed class WorkspaceCategory : SimpleCategory
    {
        private readonly string text;

        public WorkspaceCategory(string text, List<IAutoCompleteListItem> children)
            : base("", children)
        {
            this.text = text;
        }

        protected override UIElement createElement()
        {
            return new WPF.AutoComplete.WorkspaceCategory(this.text);
        }
    }
}