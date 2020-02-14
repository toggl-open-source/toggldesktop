using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.AutoCompletion.Implementation
{
    static class AutoCompleteControllers
    {
        public static IAutoCompleteController ForTimer(IEnumerable<Toggl.TogglAutocompleteView> items)
        {
            var list = items.Select(i => new TimerItem(i, (i.Type == 2))).ToList<IAutoCompleteListItem>();
            return new AutoCompleteController(list, $"Timer items ({list.Count})", 0);
        }

        public static IAutoCompleteController ForTags(IEnumerable<string> items)
        {
            var list = items.Select(i => new StringItem(i)).ToList<IAutoCompleteListItem>();
            var ac = new AutoCompleteController(list, $"Tags({list.Count})", 5);
            return ac;
        }
        public static IAutoCompleteController ForStrings(IEnumerable<string> items)
        {
            var list = items.Select(i => new StringItem(i)).ToList<IAutoCompleteListItem>();
           var ac = new AutoCompleteController(list, $"Strings({list.Count})", 1);
           return ac;
        }

        public static IAutoCompleteController ForProjects(List<Toggl.TogglAutocompleteView> projects)
        {
            var list = projects.Select(i => new TimerItem(i, true)).ToList<IAutoCompleteListItem>();
            var ac = new AutoCompleteController(list, $"Projects({projects.Count})", 3);
            return ac;
        }

        public static IAutoCompleteController ForClients(List<Toggl.TogglGenericView> clients)
        {
            var list = clients.Select(m => new ModelItem(m))
                .Cast<IAutoCompleteListItem>().ToList();
            var ac = new AutoCompleteController(list, $"Clients({clients.Count})", 2);
            return ac;
        }

        public static IAutoCompleteController ForWorkspaces(List<Toggl.TogglGenericView> list)
        {
            var items = list.Select(m => new ModelItem(m))
                .Cast<IAutoCompleteListItem>().ToList();
            var ac = new AutoCompleteController(items, $"Workspaces({list.Count})", 4);
            return ac;
        }
    }
}