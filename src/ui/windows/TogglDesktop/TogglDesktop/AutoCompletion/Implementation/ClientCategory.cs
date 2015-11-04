using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    sealed class ClientCategory : NestedCategory
    {
        public ClientCategory(string text, List<IAutoCompleteListItem> children)
            : base(text, children)
        {
        }

        protected override UIElement createElement(out Panel childrenPanel, List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<AutoCompleteControls.ClientCategory>()
                .Initialised(this.Text, out childrenPanel)
                .MarkForRecycling(recyclables);
        }
    }
}