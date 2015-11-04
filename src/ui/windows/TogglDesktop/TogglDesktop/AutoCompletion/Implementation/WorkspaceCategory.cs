using System.Collections.Generic;
using System.Windows;
using TogglDesktop.AutoCompleteControls;

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

        protected override UIElement createElement(List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<AutoCompleteControls.WorkspaceCategory>()
                .Initialised(this.text)
                .MarkForRecycling(recyclables);
        }
    }
}