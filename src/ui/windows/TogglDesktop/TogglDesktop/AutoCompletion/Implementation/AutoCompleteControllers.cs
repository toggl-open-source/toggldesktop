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

        public static AutoCompleteController ForProjects(
            List<Toggl.AutocompleteItem> projects, List<Toggl.Model> clients, List<Toggl.Model> workspaces)
        {
            var workspaceLookup = workspaces.ToDictionary(w => w.ID);
            foreach (var client in clients)
            {
                Console.WriteLine(client.WID + " " + client.Name + " " + client.ID + " " + client.GUID);
            }
            var clientLookup = clients.GroupBy(c => c.WID).ToDictionary(
                c => c.Key, cs => cs.ToDictionary(c => c.Name)
                );

            Func<Toggl.AutocompleteItem, Toggl.Model> getClientOfProject =
                p =>
                {
                    Toggl.Model client = default(Toggl.Model);
                    if (p.ClientLabel != null)
                    {
                        clientLookup[p.WorkspaceID].TryGetValue(p.ClientLabel, out client);
                    }
                    return client;
                };

            // categorise by workspace and client
            var list = ((IAutoCompleteListItem)new NoProjectItem())
                .Prepend(projects
                    .Where(p => p.ProjectID != 0) // TODO: get rid of these at an earlier stage (they are workspace entries which are not needed anymore)
                    .GroupBy(p => p.WorkspaceID)
                    .Select(ps => new WorkspaceCategory(
                        workspaceLookup[ps.Key].Name,
                        ps.GroupBy(p => getClientOfProject(p).ID)
                            .Select(c =>
                            {
                                var projectItems = c.Select(p2 => new ProjectItem(p2));
                                if (c.Key == 0)
                                    return projectItems;
                                var clientName = c.First().ClientLabel;
                                return new ClientCategory(clientName, projectItems.Cast<IAutoCompleteListItem>().ToList()).Yield<IAutoCompleteListItem>();
                            })
                            .SelectMany(i => i).ToList()
                        ))
                    ).ToList();

            return new AutoCompleteController(list);
        }

        public static AutoCompleteController ForDescriptions(List<Toggl.AutocompleteItem> items)
        {
            var list = items.Select(i => new DescriptionItem(i)).Cast<IAutoCompleteListItem>().ToList();

            // TODO: categorize by workspace/client/project?

            return new AutoCompleteController(list);
        }

        public static AutoCompleteController ForClients(
            List<Toggl.Model> clients, List<Toggl.Model> workspaces)
        {
            var workspaceLookup = workspaces.ToDictionary(w => w.ID);

            // categorise by workspace
            var list =
                ((IAutoCompleteListItem)new NoClientItem()).Prepend(
                    clients.GroupBy(c => c.WID).Select(
                        cs =>
                            new WorkspaceCategory(workspaceLookup[cs.Key].Name, cs.Select(
                                c => new ModelItem(c)).Cast<IAutoCompleteListItem>().ToList()
                                )
                    )
                ).ToList();

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