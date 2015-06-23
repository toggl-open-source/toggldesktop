using System.Collections.Generic;
using System.Linq;
using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class WorkspaceAutoCompleteController : AutoCompleteController
    {
        private WorkspaceAutoCompleteController(List<IAutoCompleteListItem> items)
            : base(items)
        {
        }

        public static AutoCompleteController From(List<Toggl.Model> list)
        {
            var items = list.Select(m => new ModelAutoCompleteItem(m))
                .Cast<IAutoCompleteListItem>().ToList();

            return new WorkspaceAutoCompleteController(items);
        }
    }
}