using System.Collections.Generic;
using System.Linq;
using TogglDesktop.WPF;

namespace TogglDesktop.AutoCompletion.Implementation
{
    class ClientAutoCompleteController : AutoCompleteController
    {
        private ClientAutoCompleteController(List<IAutoCompleteListItem> items)
            : base(items)
        {
        }

        public static AutoCompleteController From(List<Toggl.Model> list)
        {
            var items = list.Select(m => new ClientAutoCompleteItem(m))
                .Cast<IAutoCompleteListItem>().ToList();

            return new ClientAutoCompleteController(items);
        }
    }
}