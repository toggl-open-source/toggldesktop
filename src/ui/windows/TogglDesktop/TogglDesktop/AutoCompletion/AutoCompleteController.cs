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
        private List<ListBoxItemViewModel> items;
        public List<ListBoxItemViewModel> visibleItems;
        private ListBox _listBox;
        private ListBox LB
        {
            get => _listBox;
            set
            {
                if (_listBox != value)
                {
                    _listBox = value;
                    if (autocompleteType == 2 || autocompleteType == 3)
                    {
                        _listBox.SetValue(VirtualizingPanel.ScrollUnitProperty, ScrollUnit.Pixel);
                    }
                }
            }
        }

        public string DebugIdentifier { get; }

        public bool IsFullMatch { get; private set; }

        private int selectedIndex;
        private string filterText;
        private string[] words;
        public int autocompleteType = 0;

        private Dictionary<string, TagItemViewModel> _tagsDictionary;

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
                    if (!listitem.IsModelItem())
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

        private static List<ListBoxItemViewModel> CreateItemViewModelsList(IList<IAutoCompleteListItem> modelsList, int autocompleteType)
        {
            int lastType = -1;
            string lastClient = null;
            int lastWID = -1;
            bool noProjectAdded = false;

            var items = new List<ListBoxItemViewModel>();

            var multipleWorkspaces = false;
            for (var count = 0; count < modelsList.Count; ++count)
            {
                var item = modelsList[count];
                var it = (TimerItem)item;

                // Add workspace title
                if (lastWID != (int)it.Item.WorkspaceID)
                {
                    if (lastWID != -1) // workspace separator
                    {
                        items.Add(WorkspaceSeparatorItemViewModel.Instance);
                        multipleWorkspaces = true;
                    }

                    items.Add(new WorkspaceItemViewModel(it.Item.WorkspaceName));
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
                        items.Add(new CategoryItemViewModel(categories[(int) it.Item.Type]));
                    }

                    // if projects autocomplete show 'no project' item
                    if (autocompleteType == 3 && (int)it.Item.Type == 2
                        && !noProjectAdded)
                    {
                        items.Add(NoProjectItemViewModel.Instance);
                        noProjectAdded = true;
                    }
                    lastType = (int)it.Item.Type;
                }

                // Add client item if needed
                if (it.Item.Type == 2 && lastClient != it.Item.ClientLabel)
                {
                    items.Add(new ClientItemViewModel(it.Item.ClientLabel));
                    lastClient = it.Item.ClientLabel;
                }

                items.Add(new TimeEntryItemViewModel(it.Item, count));
            }

            if (modelsList.Count == 0 && autocompleteType == 3)
            {
                items.Add(new CustomTextItemViewModel("There are no projects yet", "Go ahead and create your first project now"));
            }

            if (!multipleWorkspaces)
            {
                // remove workspace item if there is only one workspace
                var workspaceItemIndex = items.FindIndex(x => x.Type == ItemType.WORKSPACE);
                if (workspaceItemIndex >= 0)
                    items.RemoveAt(workspaceItemIndex);
            }

            return items;
        }

        private List<ListBoxItemViewModel> CreateTagItemViewModelsList(List<IAutoCompleteListItem> modelsList)
        {
            _tagsDictionary = new Dictionary<string, TagItemViewModel>();
            return modelsList
                .Cast<StringItem>()
                .Select((item1, ind) =>
                {
                    var tagItemViewModel = new TagItemViewModel(item1.Item, ind);
                    _tagsDictionary.Add(item1.Item, tagItemViewModel);
                    return (ListBoxItemViewModel) tagItemViewModel;
                })
                .AppendIfEmpty(() =>
                    new CustomTextItemViewModel("There are no tags yet",
                    "Start typing and press Enter to add a new tag"))
                .ToList();
        }

        public void FillList(ListBox listBox)
        {
            LB = listBox;
            IsFullMatch = false;

            using (Performance.Measure("FILLIST, {0} items", this.list.Count))
            {
                items = autocompleteType switch
                {
                    // autotracker terms
                    1 => list.Select((item1, ind) => (ListBoxItemViewModel)new StringItemViewModel(((StringItem) item1).Item, ind)).ToList(),
                    // client dropdown
                    2 => list.Select((item1, ind) => (ListBoxItemViewModel)new StringItemViewModel(((ModelItem) item1).Item.Name, ind))
                        .AppendIfEmpty(() => new CustomTextItemViewModel("There are no clients yet", "Add client name and press Enter to add it as a client"))
                        .ToList(),
                    // workspace dropdown
                    4 => list.Select((item1, ind) => (ListBoxItemViewModel)new StringItemViewModel(((ModelItem) item1).Item.Name, ind)).ToList(),
                    // tags dropdown
                    5 => CreateTagItemViewModelsList(list),
                    // description and project dropdowns
                    _ => CreateItemViewModelsList(list, autocompleteType)
                };

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
                var filteredItems = new List<ListBoxItemViewModel>();
                
                if (autocompleteType != 0 && autocompleteType != 3)
                {
                    filteredItems = visibleItems.Where(FilterSimpleItem).ToList();
                }
                else
                {
                    var lastType = ItemType.CATEGORY; // ?
                    string lastProjectLabel = null;
                    string lastClient = null;
                    string lastWSName = null;
                    IsFullMatch = false;
                    foreach (var item in visibleItems.OfType<TimeEntryItemViewModel>().Where(Filter))
                    {
                        // Add workspace title
                        if (lastWSName != item.WorkspaceName)
                        {
                            filteredItems.Add(new WorkspaceItemViewModel(item.WorkspaceName));
                            lastWSName = item.WorkspaceName;
                            lastType = ItemType.CATEGORY; // WORKSPACE?
                            lastClient = null;
                        }

                        // Add category title if needed
                        if (autocompleteType == 0 && lastType != item.Type
                                                  && item.Type != ItemType.TASK)
                        {
                            filteredItems.Add(new CategoryItemViewModel(categories[(int)item.Type]));
                            lastType = item.Type;
                        }

                        // Add client item if needed
                        if ((item.Type == ItemType.PROJECT || item.Type == ItemType.TASK) && lastClient != item.ClientLabel)
                        {
                            filteredItems.Add(new ClientItemViewModel(item.ClientLabel));
                            lastClient = item.ClientLabel;
                        }

                        // In case we have task and project is not completed
                        if (item.Type == ItemType.TASK && item.ProjectLabel != lastProjectLabel)
                        {
                            filteredItems.Add(new ProjectItemViewModel(item, filteredItems.Count));
                        }

                        filteredItems.Add(item);
                        lastProjectLabel = item.ProjectLabel;
                    }
                }

                if (filteredItems.Count == 0)
                {
                    if (autocompleteType == 2)
                    {
                        filteredItems.Add(new CustomTextItemViewModel("No matching clients found", "Press Enter to add it as a client"));
                    }
                    else if (autocompleteType == 3)
                    {
                        filteredItems.Add(new CustomTextItemViewModel("No matching projects", "Try a different keyword or create a new project"));
                    }
                    else if (autocompleteType == 5)
                    {
                        filteredItems.Add(new CustomTextItemViewModel("No matching tags", "Press Enter to add it as a tag"));
                    }
                }
                else if (filteredItems.Count == 1)
                {
                    IsFullMatch = IsFullMatch || (filteredItems[0].Text == filterText);
                }

                visibleItems = filteredItems;
            }
            LB.ItemsSource = visibleItems;
            if (autocompleteType == 3)
                this.selectFirstItem(0);
        }

        private bool Filter(TimeEntryItemViewModel timeEntryItem)
        {
            var itemText = (timeEntryItem.Type == ItemType.TASK)
                ? timeEntryItem.ProjectAndTaskLabel
                : timeEntryItem.Text;

            return words.All(word => itemText.IndexOf(word, StringComparison.OrdinalIgnoreCase) != -1);
        }

        private bool FilterSimpleItem(ListBoxItemViewModel item)
        {
            return words.All(word => item.Text.IndexOf(word, StringComparison.OrdinalIgnoreCase) != -1);
        }

        private void selectFirstItem(int index)
        {
            if (this.visibleItems.Count == 0 || index >= this.visibleItems.Count)
                return;

            if (this.visibleItems[index].IsSelectable() == false)
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
            if (this.visibleItems == null || this.visibleItems.Count == 0) return;
            var maxIterations = visibleItems.Count;
            var initialIndex = this.selectedIndex % visibleItems.Count;
            var nextIndex = initialIndex;
            for (var i = 0; i < maxIterations; i++)
            {
                nextIndex = (nextIndex + 1) % this.visibleItems.Count;
                if (visibleItems[nextIndex].IsSelectable())
                {
                    this.selectIndex(nextIndex);
                    return;
                }
            }

            if (visibleItems[initialIndex].IsSelectable())
            {
                this.selectIndex(initialIndex);
            }
        }

        public void SelectPrevious()
        {
            if (this.visibleItems == null || this.visibleItems.Count == 0) return;
            var maxIterations = visibleItems.Count;
            var initialIndex = this.selectedIndex % visibleItems.Count;
            var nextIndex = initialIndex;
            for (var i = 0; i < maxIterations; i++)
            {
                nextIndex = (nextIndex - 1 + this.visibleItems.Count) % this.visibleItems.Count;
                if (visibleItems[nextIndex].IsSelectable())
                {
                    this.selectIndex(nextIndex);
                    return;
                }
            }

            if (visibleItems[initialIndex].IsSelectable())
            {
                this.selectIndex(initialIndex);
            }
        }

        public void AddTag(string tag)
        {
            if (autocompleteType != 5) throw new InvalidOperationException(nameof(AddTag) + "called on a non-tag controller");
            if (_tagsDictionary == null) throw new InvalidOperationException(nameof(AddTag) + "called on a non-initialized controller");
            if (_tagsDictionary.TryGetValue(tag, out var tagItemViewModel))
            {
                tagItemViewModel.IsChecked = true;
            }
        }

        public void RemoveTag(string tag)
        {
            if (autocompleteType != 5) throw new InvalidOperationException(nameof(RemoveTag) + "called on a non-tag controller");
            if (_tagsDictionary == null) throw new InvalidOperationException(nameof(RemoveTag) + "called on a non-initialized controller");
            if (_tagsDictionary.TryGetValue(tag, out var tagItemViewModel))
            {
                tagItemViewModel.IsChecked = false;
            }
        }
    }

    public enum ItemType
    {
        TIMEENTRY = 0,
        TASK = 1,
        PROJECT = 2,
        STRINGITEM = 4,
        TAGITEM = 5,

        CATEGORY = -1,
        CLIENT = -2,
        WORKSPACE = -3,
        WORKSPACE_SEPARATOR = -4,
        CUSTOM_TEXT = -5
    }

    public class AutocompleteTemplateSelector : DataTemplateSelector
    {
        private static readonly Dictionary<ItemType, string> DataTemplateMap = new Dictionary<ItemType, string>
        {
            {ItemType.PROJECT, "project-item-template"},
            {ItemType.TASK, "task-item-template"},
            {ItemType.TIMEENTRY, "timer-item-template"},
            {ItemType.CATEGORY, "category-item-template"},
            {ItemType.STRINGITEM, "string-item-template"},
            {ItemType.TAGITEM, "tag-item-template"},
            {ItemType.CLIENT, "client-item-template"},
            {ItemType.WORKSPACE, "workspace-item-template"},
            {ItemType.WORKSPACE_SEPARATOR, "workspace-separator-item-template"},
            {ItemType.CUSTOM_TEXT, "custom-text-item-template"},
        };
        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            return
                item is ListBoxItemViewModel listItem && DataTemplateMap.TryGetValue(listItem.Type, out var resourceKey)
                    ? ((FrameworkElement) container).FindResource(resourceKey) as DataTemplate
                    : null;
        }
    }
}