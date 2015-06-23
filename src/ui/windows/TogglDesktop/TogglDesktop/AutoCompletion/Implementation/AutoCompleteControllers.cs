using System;
using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.AutoCompletion.Implementation
{
    static class AutoCompleteControllers
    {
        public static AutoCompleteController ForStrings(IEnumerable<string> items, Func<string, bool> ignoreTag)
        {
            var list = items.Select(i => new StringItem(i, ignoreTag)).Cast<IAutoCompleteListItem>().ToList();

            return new AutoCompleteController(list);
        }

        public static AutoCompleteController ForProjects(List<Toggl.AutocompleteItem> items)
        {
            var list = items.Select(i => new ProjectItem(i)).Cast<IAutoCompleteListItem>().ToList();

            // TODO: categorize by workspace/client?

            return new AutoCompleteController(list);
        }

        public static AutoCompleteController ForDescriptions(List<Toggl.AutocompleteItem> items)
        {
            var list = items.Select(i => new DescriptionItem(i)).Cast<IAutoCompleteListItem>().ToList();

            // TODO: categorize by workspace/client/project?

            return new AutoCompleteController(list);
        }

        public static AutoCompleteController ForClients(List<Toggl.Model> clients, List<Toggl.Model> workspaces)
        {
            var workspaceLookup = workspaces.ToDictionary(w => w.ID);

            var list = clients.GroupBy(c => c.WID).Select(
                cs =>
                    new WorkspaceCategory(workspaceLookup[cs.Key].Name, cs.Select(
                        c => new ModelItem(c)).Cast<IAutoCompleteListItem>().ToList()
                        )
                ).Cast<IAutoCompleteListItem>().ToList();

            return new AutoCompleteController(list);
        }

        public static AutoCompleteController ForWorkspaces(List<Toggl.Model> list)
        {
            var items = list.Select(m => new ModelItem(m))
                .Cast<IAutoCompleteListItem>().ToList();

            return new AutoCompleteController(items);
        }
    }
}