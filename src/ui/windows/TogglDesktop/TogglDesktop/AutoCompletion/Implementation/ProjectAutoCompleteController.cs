using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ProjectAutoCompleteController : AutoCompleteController
    {
        private ProjectAutoCompleteController(List<IAutoCompleteListItem> list)
            : base(list)
        {
        }

        public static AutoCompleteController From(List<Toggl.AutocompleteItem> items)
        {
            var list = items.Select(i => new ProjectAutoCompleteItem(i)).Cast<IAutoCompleteListItem>().ToList();

            // TODO: categorize by workspace/client?

            return new ProjectAutoCompleteController(list);
        }
    }
}