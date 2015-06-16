using System;
using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class DescriptionAutoCompleteController : AutoCompleteController
    {
        private DescriptionAutoCompleteController(List<IAutoCompleteListItem> list)
            : base(list)
        {
        }

        public static AutoCompleteController From(List<Toggl.AutocompleteItem> items)
        {
            var list = items.Select(i => new DescriptionAutoCompleteItem(i)).Cast<IAutoCompleteListItem>().ToList();

            // TODO: categorize by workspace/client/project?

            return new DescriptionAutoCompleteController(list);
        }
    }
}