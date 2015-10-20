using System.Collections.Generic;
using System.Windows;
using TogglDesktop.AutoCompleteControls;

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

        protected override UIElement createElement(List<IRecyclable> recyclables)
        {
            return StaticObjectPool.PopOrNew<GrayTextCategory>()
                .Initialised(this.text)
                .MarkForRecycling(recyclables);
        }
    }
}