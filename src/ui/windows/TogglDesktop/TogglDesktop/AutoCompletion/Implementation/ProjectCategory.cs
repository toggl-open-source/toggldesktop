using System;
using System.Collections.Generic;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    public struct CountedAutoCompleteView
    {
        public readonly int Count;
        public readonly Toggl.TogglAutocompleteView View;

        public CountedAutoCompleteView(int count, Toggl.TogglAutocompleteView view)
        {
            this.Count = count;
            this.View = view;
        }
    }

    class ProjectCategory : SimpleItemCategory<TogglDesktop.AutoCompleteControls.ProjectCategory, CountedAutoCompleteView>, IProjectItem
    {
        public ProjectCategory(CountedAutoCompleteView item, List<IAutoCompleteListItem> children)
            : base(item, createAutocompletionString(item), children)
        {
        }

        public new Toggl.TogglAutocompleteView Item { get { return base.Item.View; } }

        private static string createAutocompletionString(CountedAutoCompleteView item)
        {
            var v = item.View;

            return string.IsNullOrEmpty(v.ClientLabel)
                ? v.ProjectLabel
                : v.ProjectLabel + " " + v.ClientLabel;
        }

        protected override TogglDesktop.AutoCompleteControls.ProjectCategory createElement(
            Action selectWithClick, List<IRecyclable> recyclables,
            out Panel newParent, out ICollapsable collapsable)
        {
            var element = StaticObjectPool.PopOrNew<TogglDesktop.AutoCompleteControls.ProjectCategory>()
                .Initialised(base.Item, selectWithClick)
                .MarkForRecycling(recyclables);
            newParent = element.TaskPanel;
            collapsable = element;

            return element;
        }
    }
}