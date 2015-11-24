using System;
using System.Collections.Generic;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ProjectCategory : SimpleItemCategory<TogglDesktop.AutoCompleteControls.ProjectCategory, Toggl.TogglAutocompleteView>
    {
        public ProjectCategory(Toggl.TogglAutocompleteView item, List<IAutoCompleteListItem> children)
            : base(item, createAutocompletionString(item), children)
        {
        }


        private static string createAutocompletionString(Toggl.TogglAutocompleteView item)
        {
            return string.IsNullOrEmpty(item.ClientLabel)
                ? item.ProjectLabel
                : item.ProjectLabel + " " + item.ClientLabel;
        }

        protected override TogglDesktop.AutoCompleteControls.ProjectCategory createElement(
            Action selectWithClick, List<IRecyclable> recyclables, out Panel newParent)
        {
            var element = StaticObjectPool.PopOrNew<TogglDesktop.AutoCompleteControls.ProjectCategory>()
                .Initialised(this.Item, selectWithClick)
                .MarkForRecycling(recyclables);
            newParent = element.TaskPanel;

            return element;
        }
    }
}