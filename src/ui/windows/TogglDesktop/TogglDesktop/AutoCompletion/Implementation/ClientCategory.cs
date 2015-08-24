using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.WPF.AutoComplete;

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
            return StaticObjectPool.PopOrNew<WPF.AutoComplete.ClientCategory>()
                .Initialised(this.Text, out childrenPanel)
                .Recycle(recyclables);
        }
    }
}