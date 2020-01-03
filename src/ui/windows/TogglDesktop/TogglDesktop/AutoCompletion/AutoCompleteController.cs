using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using TogglDesktop.AutoCompleteControls;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;

namespace TogglDesktop.AutoCompletion
{
    class AutoCompleteController
    {
        private static readonly char[] splitChars = { ' ' };

        private string[] categories = { "TIME ENTRIES", "TASKS", "PROJECTS", "WORKSPACES", "TAGS" };

        private readonly List<IAutoCompleteListItem> list;
        private List<ListBoxItem> items;
        public List<ListBoxItem> visibleItems;
        private ListBox LB;
        public string DebugIdentifier { get; private set; }

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
                    if (listitem.Type == -1)
                    {
                        return null;
                    }
                    // no project item
                    if (listitem.Index == -1)
                    {
                        return (AutoCompleteItem)new TimerItem(new Toggl.TogglAutocompleteView(), true);
                    }
                    return (AutoCompleteItem)this.list[listitem.Index];
                }
                return null;
            }
        }

        public List<IAutoCompleteListItem> getList()
        {
            return this.list;
        }

        public void FillList(ListBox listBox, Action<AutoCompleteItem> selectWithClick, List<IRecyclable> recyclables)
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
                            Type = 4,
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
                            Type = 4,
                            Index = count
                        });
                    }
                }
                // description and project dropdowns
                else
                {
                    for (var count = 0; count < this.list.Count; ++count)
                    {
                        var item = this.list[count];
                        var it = (TimerItem)item;

                        // Add workspace title
                        if (lastWID != (int)it.Item.WorkspaceID)
                        {
                            items.Add(new ListBoxItem()
                            {
                                Text = it.Item.WorkspaceName.ToUpper(),
                                Type = -3
                            });
                            lastWID = (int)it.Item.WorkspaceID;
                            lastType = -1;
                            lastClient = null;
                        }

                        // Add category title if needed
                        if (lastType != (int)it.Item.Type && (int)it.Item.Type != 1)
                        {
                            items.Add(new ListBoxItem()
                            {
                                Category = categories[(int)it.Item.Type],
                                Type = -1
                            });

                            // if projects autocomplete show 'no project' item
                            if (autocompleteType == 3 && (int)it.Item.Type == 2
                                && !noProjectAdded)
                            {
                                items.Add(new ListBoxItem()
                                {
                                    Text = "No project",
                                    Description = "No project",
                                    ProjectLabel = "",
                                    TaskLabel = "",
                                    ClientLabel = "",
                                    Type = 0,
                                    Index = -1
                                });
                                noProjectAdded = true;
                            }
                            lastType = (int)it.Item.Type;
                        }

                        // Add client item if needed
                        if (it.Item.Type == 2 && lastClient != it.Item.ClientLabel)
                        {
                            string text = it.Item.ClientLabel;
                            if (text.Length == 0)
                            {
                                text = "No client";
                            }
                            items.Add(new ListBoxItem()
                            {
                                Text = text,
                                Type = -2
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
                            Type = (int)it.Item.Type,
                            WorkspaceName = it.Item.WorkspaceName.ToUpper(),
                            Index = count
                        });
                    }
                }
                visibleItems = items;
                LB.ItemsSource = visibleItems;
            }
        }

        public void Complete(string input)
        {
            if (String.IsNullOrEmpty(input))
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

                int lastType = -1;
                string lastProjectLabel = null;
                string lastClient = null;
                string lastWSName = null;
                List<ListBoxItem> filteredItems = new List<ListBoxItem>();
                foreach (var item in visibleItems)
                {
                    if (Filter(item))
                    {
                        // Add workspace title
                        if (lastWSName != item.WorkspaceName)
                        {
                            filteredItems.Add(new ListBoxItem()
                            {
                                Text = item.WorkspaceName,
                                Type = -3
                            });
                            lastWSName = item.WorkspaceName;
                            lastType = -1;
                            lastClient = null;
                        }

                        // Add category title if needed
                        if (autocompleteType == 0 && lastType != (int)item.Type
                            && (int)item.Type != 1)
                        {
                            filteredItems.Add(new ListBoxItem() {
                                Category = categories[(int)item.Type],
                                Type = -1
                            });
                            lastType = (int)item.Type;
                        }

                        // Add client item if needed
                        if ((item.Type == 2 || item.Type == 1) && lastClient != item.ClientLabel)
                        {
                            string text = item.ClientLabel;
                            if (text.Length == 0)
                            {
                                text = "No client";
                            }
                            filteredItems.Add(new ListBoxItem()
                            {
                                Text = text,
                                Type = -2
                            });
                            lastClient = item.ClientLabel;
                        }

                        // In case we have task and project is not completed
                        if (item.Type == 1 && item.ProjectLabel != lastProjectLabel)
                        {
                            filteredItems.Add(new ListBoxItem()
                            {
                                Text = item.ProjectLabel,
                                Description = "",
                                ProjectLabel = item.ProjectLabel,
                                ProjectColor = item.ProjectColor,
                                TaskLabel = "",
                                ClientLabel = item.ClientLabel,
                                Type = 2,
                                WorkspaceName = item.WorkspaceName,
                                Index = filteredItems.Count
                            });
                        }

                        filteredItems.Add(item);
                        lastProjectLabel = item.ProjectLabel;
                    }
                }
                visibleItems = filteredItems;
            }
            LB.ItemsSource = visibleItems;
            if (autocompleteType == 3)
                this.selectFirstItem(0);
        }

        private bool Filter(object item)
        {
            if (String.IsNullOrEmpty(filterText))
                return true;

            var listItem = (ListBoxItem)item;

            if (listItem.Type < 0)
                return false;

            string itemText = (listItem.Type == 1) ? listItem.ProjectAndTaskLabel : listItem.Text;

            foreach (string word in words)
            {
                if (itemText.IndexOf(word, StringComparison.OrdinalIgnoreCase) == -1)
                {
                    return false;
                }
            }
            return true;
        }


        public void RefreshVisibleList()
        {
            this.completeWith(i => i.CompleteVisible());
        }

        private void completeWith(Func<IAutoCompleteListItem, IEnumerable<AutoCompleteItem>> completor)
        {
            this.validateSelection();

        }

        private void validateSelection()
        {
            if (this.selectedIndex == -1)
                return;
            /* old
            var selectedItemIndex = this.visibleItems.IndexOf(this.items[LB.SelectedIndex]);
            this.selectIndex(selectedItemIndex);
             * */
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

        public void SelectItem(AutoCompleteItem item)
        {
            /*old
            var i = item == null ? -1 : this.visibleItems.IndexOf(item);
            this.selectIndex(i);
             * */
        }

        public bool TryCollapseCategory()
        {
            var asItemCategory = this.list[visibleItems[LB.SelectedIndex].Index] as AutoCompleteItemCategory;
            if (asItemCategory == null || asItemCategory.Collapsed)
                return false;

            asItemCategory.Collapsed = true;
            this.RefreshVisibleList();
            return true;
        }

        public bool TryExpandCategory()
        {
            var asItemCategory = this.list[visibleItems[LB.SelectedIndex].Index] as AutoCompleteItemCategory;
            if (asItemCategory == null || !asItemCategory.Collapsed)
                return false;

            asItemCategory.Collapsed = false;
            this.RefreshVisibleList();
            return true;
        }
    }

    public class AutocompleteTemplateSelector : DataTemplateSelector
    {
        private const int CATEGORY = -1;
        private const int CLIENT = -2;
        private const int TIMEENTRY = 0;
        private const int TASK = 1;
        private const int PROJECT = 2;
        private const int WORKSPACE = -3;
        private const int STRINGITEM = 4;

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            FrameworkElement element = container as FrameworkElement;

            var listItem = item as ListBoxItem;
            if (listItem == null)
                return null;

            if (listItem.Type == PROJECT)
            {
                return
                    element.FindResource("project-item-template")
                    as DataTemplate;
            }
            else if (listItem.Type == TASK)
            {
                return
                    element.FindResource("task-item-template")
                    as DataTemplate;
            }
            else if (listItem.Type == TIMEENTRY)
            {
                return
                    element.FindResource("timer-item-template")
                    as DataTemplate;
            }
            else if (listItem.Type == CATEGORY)
            {
                return
                    element.FindResource("category-item-template")
                    as DataTemplate;
            }
            else if (listItem.Type == STRINGITEM)
            {
                return
                    element.FindResource("string-item-template")
                    as DataTemplate;
            }
            else if (listItem.Type == CLIENT)
            {
                return
                    element.FindResource("client-item-template")
                    as DataTemplate;
            }
            else if (listItem.Type == WORKSPACE)
            {
                return
                    element.FindResource("workspace-item-template")
                    as DataTemplate;
            }

            return null;
        }
    }
}