using System;
using System.Collections.Generic;
using System.Linq;
using TogglDesktop.AutoCompletion.Items;

namespace TogglDesktop.AutoCompletion
{
    static class AutoCompleteControllersFactory
    {
        public static IAutoCompleteController ForTimer(IList<Toggl.TogglAutocompleteView> items)
        {
            return new AutoCompleteController(items, $"Timer items ({items.Count})", 0);
        }

        public static TagsAutoCompleteController ForTags(IEnumerable<string> items, Func<string, bool> isSelected)
        {
            return new TagsAutoCompleteController(items, isSelected);
        }

        public static IAutoCompleteController ForProjects(List<Toggl.TogglAutocompleteView> projects)
        {
            return new AutoCompleteController(projects, $"Projects({projects.Count})", 3);
        }

        public static IAutoCompleteController ForClients(List<Toggl.TogglGenericView> clients)
        {
            var list = clients
                .Select(client => new GenericModelItem(client))
                .ToList<IAutoCompleteItem>();
            return new AutoCompleteController(list, $"Clients({clients.Count})", 2);
        }
    }
}