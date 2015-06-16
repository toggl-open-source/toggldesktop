using System;
using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class DescriptionAutoCompleteController : AutoCompleteController
    {
        private DescriptionAutoCompleteController(List<IAutoCompleteListItem> list, IReadOnlyCollection<AutoCompleteItem> items)
            : base(list, items)
        {
        }

        public static AutoCompleteController From(List<Toggl.AutocompleteItem> items)
        {
            var newItems = items.Select(i => new DescriptionAutoCompleteItem(i)).ToList();

            // TODO: categorize by workspace/client/project?

            var list = newItems.Cast<IAutoCompleteListItem>().ToList();

            return new DescriptionAutoCompleteController(list, newItems);
        }
    }
}