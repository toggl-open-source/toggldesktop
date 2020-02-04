using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;

namespace TogglDesktop.AutoCompletion
{
    class AutoCompleteController
    {
        private static readonly char[] splitChars = { ' ' };

        private static readonly string[] categories = { "RECENT TIME ENTRIES", "TASKS", "PROJECTS", "WORKSPACES", "TAGS" };

        private readonly List<IAutoCompleteListItem> list;
        private List<ListBoxItem> items;
        public List<ListBoxItem> visibleItems;
        private ListBox LB;
        public string DebugIdentifier { get; }

        private int selectedIndex;
        private string filterText;
        private string[] words;
        public int autocompleteType = 0;

        public AutoCompleteController(List<IAutoCompleteListItem> list, string debugIdentifier)
        {
            this.list = list;
            this.DebugIdentifier = debugIdentifier;
        }

        public AutoCompleteItem SelectedItem
        {
            get {
                if (LB != null && LB.SelectedIndex != -1) {
                    var listitem = visibleItems[LB.SelectedIndex];
                    if (listitem.Type < 0)
                    {
                        return null;
                    }
                    // no project item
                    if (listitem.Index == -1)
                    {
                        return new TimerItem(new Toggl.TogglAutocompleteView(), true);
                    }
                    return (AutoCompleteItem)this.list[listitem.Index];
                }
                return null;
            }
        }

        public void FillList(ListBox listBox)
        {
            LB = listBox;
            int lastType = -1;
            string lastClient = null;
            int lastWID = -1;
            bool noProjectAdded = false;
            using (Performance.Measure("FILLIST, {0} items", this.list.Count))
            {
                items = new List<ListBoxItem>();

                // For tags and autotracker terms
                if (autocompleteType == 1)
                {
                    for (var count = 0; count < this.list.Count; ++count)
                    {
                        var item = this.list[count];
                        var it = (StringItem)item;

                        items.Add(new ListBoxItem()
                        {
                            Text = it.Item,
                            Type = ItemType.STRINGITEM,
                            Index = count
                        });
                    }
                }

                // workspace/client dropdown
                else if (autocompleteType == 2)
                {
                    for (var count = 0; count < this.list.Count; ++count)
                    {
                        var item = this.list[count];
                        var it = (ModelItem)item;

                        items.Add(new ListBoxItem()
                        {
                            Text = it.Item.Name,
                            Type = ItemType.STRINGITEM,
                            Index = count
                        });
                    }
                }
                // description and project dropdowns
                else
                {
                    var multipleWorkspaces = false;
                    for (var count = 0; count < this.list.Count; ++count)
                    {
                        var item = this.list[count];
                        var it = (TimerItem)item;

                        // Add workspace title
                        if (lastWID != (int)it.Item.WorkspaceID)
                        {
                            if (lastWID != -1) // workspace separator
                            {
                                items.Add(new ListBoxItem { Type = ItemType.WORKSPACE_SEPARATOR });
                                multipleWorkspaces = true;
                            }

                            items.Add(new ListBoxItem()
                            {
                                Text = it.Item.WorkspaceName,
                                Type = ItemType.WORKSPACE
                            });
                            lastWID = (int)it.Item.WorkspaceID;
                            lastType = -1;
                            lastClient = null;
                        }

                        // Add category title if needed
                        if (lastType != (int)it.Item.Type && (int)it.Item.Type != 1)
                        {
                            // do not show 'Projects' item when auto completing projects
                            if (autocompleteType != 3)
                            {
                                items.Add(new ListBoxItem()
                                {
                                    Category = categories[(int)it.Item.Type],
                                    Type = ItemType.CATEGORY
                                });
                            }

                            // if projects autocomplete show 'no project' item
                            if (autocompleteType == 3 && (int)it.Item.Type == 2
                                && !noProjectAdded)
                            {
                                items.Add(new ListBoxItem()
                                {
                                    Text = "No project",
                                    Description = "",
                                    ProjectLabel = "No project",
                                    TaskLabel = "",
                                    ClientLabel = "",
                                    Type = ItemType.PROJECT,
                                    Index = -1
                                });
                                noProjectAdded = true;
                            }
                            lastType = (int)it.Item.Type;
                        }

                        // Add client item if needed
                        if (it.Item.Type == 2 && lastClient != it.Item.ClientLabel)
                        {
                            var text = it.Item.ClientLabel;
                            if (text.Length == 0)
                            {
                                text = "No client";
                            }
                            items.Add(new ListBoxItem()
                            {
                                Text = text,
                                Type = ItemType.CLIENT
                            });
                            lastClient = it.Item.ClientLabel;
                        }

                        var taskLabel = it.Item.TaskLabel;
                        if (it.Item.Type == 0)
                        {
                            taskLabel = (it.Item.TaskLabel.Length > 0) ? " - " + it.Item.TaskLabel : "";
                        }
                        var clientLabel = (it.Item.ClientLabel.Length > 0) ? " " + it.Item.ClientLabel : "";

                        items.Add(new ListBoxItem()
                        {
                            Text = it.Item.Text,
                            Description = it.Item.Description,
                            ProjectLabel = it.Item.ProjectLabel,
                            ProjectColor = it.Item.ProjectColor,
                            ProjectAndTaskLabel = it.Item.ProjectAndTaskLabel,
                            TaskLabel = taskLabel,
                            ClientLabel = clientLabel,
                            Type = (ItemType)((int)it.Item.Type),
                            WorkspaceName = it.Item.WorkspaceName,
                            Index = count
                        });
                    }

                    if (!multipleWorkspaces)
                    {
                        // remove workspace item if there is only one workspace
                        items.RemoveAt(items.FindIndex(x => x.Type == ItemType.WORKSPACE));
                    }

                    if (autocompleteType == 3)
                    {
                        items.Add(new ListBoxItem()
                        {
                            Type = ItemType.CREATE_PROJECT_BUTTON
                        });
                    }
                }
                visibleItems = items;
                LB.ItemsSource = visibleItems;
            }
        }

        public void Complete(string input)
        {
            if (string.IsNullOrEmpty(input))
            {
                visibleItems = items;
            }
            else
            {
                if (filterText != null && !input.StartsWith(filterText))
                {
                    visibleItems = items;
                }
                words = input.Split(splitChars, StringSplitOptions.RemoveEmptyEntries);
                filterText = input;

                var lastType = ItemType.CATEGORY; // ?
                string lastProjectLabel = null;
                string lastClient = null;
                string lastWSName = null;
                var filteredItems = new List<ListBoxItem>();
                foreach (var item in visibleItems.Where(Filter))
                {
                    // Add workspace title
                    if (lastWSName != item.WorkspaceName)
                    {
                        filteredItems.Add(new ListBoxItem()
                        {
                            Text = item.WorkspaceName,
                            Type = ItemType.WORKSPACE
                        });
                        lastWSName = item.WorkspaceName;
                        lastType = ItemType.CATEGORY; // WORKSPACE?
                        lastClient = null;
                    }

                    // Add category title if needed
                    if (autocompleteType == 0 && lastType != item.Type
                                              && item.Type != ItemType.TASK)
                    {
                        filteredItems.Add(new ListBoxItem() {
                            Category = categories[(int)item.Type],
                            Type = ItemType.CATEGORY
                        });
                        lastType = item.Type;
                    }

                    // Add client item if needed
                    if ((item.Type == ItemType.PROJECT || item.Type == ItemType.TASK) && lastClient != item.ClientLabel)
                    {
                        var text = item.ClientLabel;
                        if (text.Length == 0)
                        {
                            text = "No client";
                        }
                        filteredItems.Add(new ListBoxItem()
                        {
                            Text = text,
                            Type = ItemType.CLIENT
                        });
                        lastClient = item.ClientLabel;
                    }

                    // In case we have task and project is not completed
                    if (item.Type == ItemType.TASK && item.ProjectLabel != lastProjectLabel)
                    {
                        filteredItems.Add(new ListBoxItem()
                        {
                            Text = item.ProjectLabel,
                            Description = "",
                            ProjectLabel = item.ProjectLabel,
                            ProjectColor = item.ProjectColor,
                            TaskLabel = "",
                            ClientLabel = item.ClientLabel,
                            Type = ItemType.PROJECT,
                            WorkspaceName = item.WorkspaceName,
                            Index = filteredItems.Count
                        });
                    }

                    filteredItems.Add(item);
                    lastProjectLabel = item.ProjectLabel;
                }
                visibleItems = filteredItems;
            }
            LB.ItemsSource = visibleItems;
            if (autocompleteType == 3)
                this.selectFirstItem(0);
        }

        private bool Filter(ListBoxItem listItem)
        {
            if (string.IsNullOrEmpty(filterText))
                return true;

            if (listItem.Type < 0)
                return false;

            var itemText = (listItem.Type == ItemType.TASK) ? listItem.ProjectAndTaskLabel : listItem.Text;

            return words.All(word => itemText.IndexOf(word, StringComparison.OrdinalIgnoreCase) != -1);
        }

        private void selectFirstItem(int index)
        {
            if (this.visibleItems.Count == 0 || index >= this.visibleItems.Count)
                return;

            if (this.visibleItems[index].Type < 0)
            {
                this.selectFirstItem(++index);
                return;
            }
            this.selectIndex(index);
        }

        private void selectIndex(int index)
        {
            if (index < 0 || this.visibleItems.Count == 0 || index >= this.visibleItems.Count)
                index = 0;

            this.selectedIndex = index;
            LB.SelectedIndex = index;

            LB.UpdateLayout();
            if (this.visibleItems.Count > 0 && LB.SelectedIndex != -1)
                LB.ScrollIntoView(LB.Items[LB.SelectedIndex]);
        }

        public void SelectNext()
        {
            if (this.visibleItems == null || this.visibleItems.Count == 0)
                return;

            var i = this.selectedIndex + 1;
            if (i >= this.visibleItems.Count)
            {
                i = 0;
                LB.UpdateLayout();
                LB.ScrollIntoView(LB.Items[0]);
            }

            if (i >= 0 && this.visibleItems[i].Type < 0)
            {
                this.selectedIndex = i;
                this.SelectNext();
                return;
            }
            this.selectIndex(i);
        }

        public void SelectPrevious()
        {
            if (this.visibleItems == null || this.visibleItems.Count == 0)
                return;

            var i = this.selectedIndex - 1;
            if (i < 0 || i >= this.visibleItems.Count)
            {
                i = this.visibleItems.Count - 1;   
            }
            
            if (this.visibleItems[i].Type < 0)
            {
                this.selectedIndex = i;
                this.SelectPrevious();
                return;
            }
            this.selectIndex(i);
        }
    }

    public enum ItemType : int
    {
        TIMEENTRY = 0,
        TASK = 1,
        PROJECT = 2,
        STRINGITEM = 4,
        CREATE_PROJECT_BUTTON = -5,

        // negative values mean non-selectable items
        CATEGORY = -1,
        CLIENT = -2,
        WORKSPACE = -3,
        WORKSPACE_SEPARATOR = -4,
    }

    public class AutocompleteTemplateSelector : DataTemplateSelector
    {
        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (!(item is ListBoxItem listItem))
                return null;

            var element = (FrameworkElement) container;
            return listItem.Type switch
            {
                ItemType.PROJECT => (element.FindResource("project-item-template") as DataTemplate),
                ItemType.TASK => (element.FindResource("task-item-template") as DataTemplate),
                ItemType.TIMEENTRY => (element.FindResource("timer-item-template") as DataTemplate),
                ItemType.CATEGORY => (element.FindResource("category-item-template") as DataTemplate),
                ItemType.STRINGITEM => (element.FindResource("string-item-template") as DataTemplate),
                ItemType.CLIENT => (element.FindResource("client-item-template") as DataTemplate),
                ItemType.WORKSPACE => (element.FindResource("workspace-item-template") as DataTemplate),
                ItemType.WORKSPACE_SEPARATOR => (element.FindResource("workspace-separator-item-template") as DataTemplate),
                ItemType.CREATE_PROJECT_BUTTON => (element.FindResource("create-project-button-item-template") as DataTemplate),
                _ => null
            };
        }
    }
}