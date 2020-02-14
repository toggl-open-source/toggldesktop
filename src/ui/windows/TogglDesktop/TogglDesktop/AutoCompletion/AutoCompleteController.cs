using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;

namespace TogglDesktop.AutoCompletion
{
    class AutoCompleteController : IAutoCompleteController
    {
        private static readonly char[] splitChars = { ' ' };
        private static readonly string[] categories = { "RECENT TIME ENTRIES", "TASKS", "PROJECTS", "WORKSPACES", "TAGS" };
        private readonly List<ListBoxItemViewModel> items;
        public IList<ListBoxItemViewModel> VisibleItems
        {
            get => _selectionManager.Items;
            private set => _selectionManager.Items = value;
        }
        private ListBox LB
        {
            get => _selectionManager.ListBox;
            set
            {
                if (_selectionManager.ListBox != value)
                {
                    _selectionManager.ListBox = value;
                    if (_autocompleteType == 2 || _autocompleteType == 3)
                    {
                        _selectionManager.ListBox.SetValue(VirtualizingPanel.ScrollUnitProperty, ScrollUnit.Pixel);
                    }
                }
            }
        }

        public string DebugIdentifier { get; }

        private string filterText;
        private string[] words;
        private readonly int _autocompleteType;
        private readonly ListBoxSelectionManager<ListBoxItemViewModel> _selectionManager = new ListBoxSelectionManager<ListBoxItemViewModel>();

        public AutoCompleteController(List<IAutoCompleteListItem> list, string debugIdentifier, int autocompleteType)
        {
            this.DebugIdentifier = debugIdentifier;
            this._autocompleteType = autocompleteType;
            items = CreateItemViewModelsList(list, autocompleteType);
            VisibleItems = items;
        }

        public AutoCompleteItem SelectedItem
        {
            get
            {
                if (LB != null && LB.SelectedIndex != -1 && VisibleItems[LB.SelectedIndex] is IModelItemViewModel modelItem)
                {
                    return modelItem.Model;
                }
                return null;
            }
        }

        private static List<ListBoxItemViewModel> CreateTimerItemViewModelsList(IList<IAutoCompleteListItem> modelsList, int autocompleteType)
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

                items.Add(new TimeEntryItemViewModel(it));
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

        private static List<ListBoxItemViewModel> CreateTagItemViewModelsList(List<IAutoCompleteListItem> modelsList)
        {
            return modelsList
                .Cast<StringItem>()
                .Select(stringItem =>
                {
                    var tagItemViewModel = new TagItemViewModel(stringItem);
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
            LB.ItemsSource = VisibleItems;
        }

        private static List<ListBoxItemViewModel> CreateItemViewModelsList(List<IAutoCompleteListItem> list, int autoCompleteType)
        {
            using (Performance.Measure("FILLIST, {0} items", list.Count))
            {
                return autoCompleteType switch
                {
                    // autotracker terms
                    1 => list.Select(item1 => (ListBoxItemViewModel)new StringItemViewModel((StringItem) item1)).ToList(),
                    // client dropdown
                    2 => list.Select(item1 => (ListBoxItemViewModel)new StringItemViewModel((ModelItem) item1))
                        .AppendIfEmpty(() => new CustomTextItemViewModel("There are no clients yet", "Add client name and press Enter to add it as a client"))
                        .ToList(),
                    // workspace dropdown
                    4 => list.Select(item1 => (ListBoxItemViewModel)new StringItemViewModel((ModelItem) item1)).ToList(),
                    // tags dropdown
                    5 => CreateTagItemViewModelsList(list),
                    // description and project dropdowns
                    _ => CreateTimerItemViewModelsList(list, autoCompleteType)
                };
            }
        }

        public void Complete(string input)
        {
            if (string.IsNullOrEmpty(input))
            {
                VisibleItems = items;
            }
            else
            {
                if (filterText != null && !input.StartsWith(filterText))
                {
                    VisibleItems = items;
                }
                words = input.Split(splitChars, StringSplitOptions.RemoveEmptyEntries);
                filterText = input;
                var filteredItems = new List<ListBoxItemViewModel>();
                
                if (_autocompleteType != 0 && _autocompleteType != 3)
                {
                    filteredItems = VisibleItems.Where(FilterSimpleItem).ToList();
                }
                else
                {
                    var lastType = ItemType.CATEGORY; // ?
                    string lastProjectLabel = null;
                    string lastClient = null;
                    string lastWSName = null;
                    foreach (var item in VisibleItems.OfType<TimeEntryItemViewModel>().Where(Filter))
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
                        if (_autocompleteType == 0 && lastType != item.Type
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
                            filteredItems.Add(new ProjectItemViewModel(item));
                        }

                        filteredItems.Add(item);
                        lastProjectLabel = item.ProjectLabel;
                    }
                }

                if (filteredItems.Count == 0)
                {
                    if (_autocompleteType == 2)
                    {
                        filteredItems.Add(new CustomTextItemViewModel("No matching clients found", "Press Enter to add it as a client"));
                    }
                    else if (_autocompleteType == 3)
                    {
                        filteredItems.Add(new CustomTextItemViewModel("No matching projects", "Try a different keyword or create a new project"));
                    }
                    else if (_autocompleteType == 5)
                    {
                        filteredItems.Add(new CustomTextItemViewModel("No matching tags", "Press Enter to add it as a tag"));
                    }
                }

                VisibleItems = filteredItems;
            }
            LB.ItemsSource = VisibleItems;
            if (_autocompleteType == 3)
                this._selectionManager.SelectFirstItem();
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

        public void SelectNext()
        {
            _selectionManager.SelectNext();
        }

        public void SelectPrevious()
        {
            _selectionManager.SelectPrevious();
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