using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using TogglDesktop.AutoCompletion.Items;

namespace TogglDesktop.AutoCompletion
{
    class AutoCompleteController : IAutoCompleteController
    {
        private static readonly char[] splitChars = { ' ' };
        private static readonly string[] categories = { "RECENT TIME ENTRIES", "TASKS", "PROJECTS", "WORKSPACES", "TAGS" };
        private readonly List<IAutoCompleteItem> _fullItemsList;
        public IList<IAutoCompleteItem> VisibleItems
        {
            get => _selectionManager.Items;
            private set => _selectionManager.Items = value;
        }

        public bool ShowActionButton
        {
            get { return _autocompleteType == 3
                         || (_autocompleteType == 2
                             && !VisibleItems.Any(it => it.Text == filterText && it.Type == ItemType.STRINGITEM)); }
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
        private readonly ListBoxSelectionManager<IAutoCompleteItem> _selectionManager = new ListBoxSelectionManager<IAutoCompleteItem>();

        public AutoCompleteController(IList<Toggl.TogglAutocompleteView> list, string debugIdentifier, int autocompleteType)
            : this(CreateTimerItemViewModelsList(list, autocompleteType), debugIdentifier, autocompleteType)
        {
        }

        public AutoCompleteController(List<IAutoCompleteItem> list, string debugIdentifier, int autocompleteType)
        {
            this.DebugIdentifier = debugIdentifier;
            this._autocompleteType = autocompleteType;
            _fullItemsList = list;
            VisibleItems = _fullItemsList;
        }

        public IAutoCompleteItem SelectedItem
        {
            get
            {
                if (LB != null && LB.SelectedIndex != -1)
                {
                    return VisibleItems[LB.SelectedIndex];
                }
                return null;
            }
        }

        private static List<IAutoCompleteItem> CreateTimerItemViewModelsList(IList<Toggl.TogglAutocompleteView> modelsList, int autocompleteType)
        {
            int lastType = -1;
            string lastClient = null;
            int lastWID = -1;
            bool noProjectAdded = false;

            var items = new List<IAutoCompleteItem>();

            var multipleWorkspaces = false;
            for (var count = 0; count < modelsList.Count; ++count)
            {
                var it = modelsList[count];

                // Add workspace title
                if (lastWID != (int)it.WorkspaceID)
                {
                    if (lastWID != -1) // workspace separator
                    {
                        items.Add(WorkspaceSeparatorItem.Instance);
                        multipleWorkspaces = true;
                    }

                    items.Add(new AutoCompleteItem(it.WorkspaceName, ItemType.WORKSPACE));
                    lastWID = (int)it.WorkspaceID;
                    lastType = -1;
                    lastClient = null;
                }

                // Add category title if needed
                if (lastType != (int)it.Type && (int)it.Type != 1)
                {
                    // do not show 'Projects' item when auto completing projects
                    if (autocompleteType != 3)
                    {
                        items.Add(new AutoCompleteItem(categories[(int) it.Type], ItemType.CATEGORY));
                    }

                    // if projects autocomplete show 'no project' item
                    if (autocompleteType == 3 && (int)it.Type == 2
                        && !noProjectAdded)
                    {
                        items.Add(NoProjectItem.Instance);
                        noProjectAdded = true;
                    }
                    lastType = (int)it.Type;
                }

                // Add client item if needed
                if (it.Type == 2 && lastClient != it.ClientLabel)
                {
                    items.Add(new AutoCompleteItem(string.IsNullOrEmpty(it.ClientLabel) ? "No client" : it.ClientLabel, ItemType.CLIENT));
                    lastClient = it.ClientLabel;
                }

                items.Add(new TimeEntryItem(it));
            }

            if (modelsList.Count == 0 && autocompleteType == 3)
            {
                items.Add(new CustomTextItem("There are no projects yet", "Go ahead and create your first project now"));
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

        public void FillList(ListBox listBox)
        {
            LB = listBox;
            LB.ItemsSource = VisibleItems;
        }

        public void Complete(string input)
        {
            if (string.IsNullOrEmpty(input))
            {
                VisibleItems = _fullItemsList;
            }
            else
            {
                if (filterText != null && !input.StartsWith(filterText))
                {
                    VisibleItems = _fullItemsList;
                }
                words = input.Split(splitChars, StringSplitOptions.RemoveEmptyEntries);
                filterText = input;
                var filteredItems = new List<IAutoCompleteItem>();
                
                if (_autocompleteType != 0 && _autocompleteType != 3)
                {
                    filteredItems = VisibleItems.Where(Filter).ToList();
                }
                else
                {
                    var lastType = ItemType.CATEGORY; // ?
                    string lastProjectLabel = null;
                    string lastClient = null;
                    string lastWSName = null;
                    var multipleWorkspaces = VisibleItems.Count > 0 && VisibleItems[0].Type == ItemType.WORKSPACE;
                    foreach (var item in VisibleItems.OfType<TimeEntryItem>().Where(Filter))
                    {
                        // Add workspace title
                        if (multipleWorkspaces && lastWSName != item.WorkspaceName)
                        {
                            if (lastWSName != null) // workspace separator
                            {
                                filteredItems.Add(WorkspaceSeparatorItem.Instance);
                            }
                            filteredItems.Add(new AutoCompleteItem(item.WorkspaceName, ItemType.WORKSPACE));
                            lastWSName = item.WorkspaceName;
                            lastType = ItemType.CATEGORY; // WORKSPACE?
                            lastClient = null;
                        }

                        // Add category title if needed
                        if (_autocompleteType == 0 && lastType != item.Type
                                                  && item.Type != ItemType.TASK)
                        {
                            filteredItems.Add(new AutoCompleteItem(categories[(int)item.Type], ItemType.CATEGORY));
                            lastType = item.Type;
                        }

                        // Add client item if needed
                        if ((item.Type == ItemType.PROJECT || item.Type == ItemType.TASK) && lastClient != item.ClientLabel)
                        {
                            filteredItems.Add(new AutoCompleteItem(string.IsNullOrEmpty(item.ClientLabel) ? "No client" : item.ClientLabel, ItemType.CLIENT));
                            lastClient = item.ClientLabel;
                        }

                        // In case we have task and project is not completed
                        if (item.Type == ItemType.TASK && item.ProjectLabel != lastProjectLabel)
                        {
                            filteredItems.Add(new ProjectItem(item));
                        }

                        filteredItems.Add(item);
                        lastProjectLabel = item.ProjectLabel;
                    }
                }

                if (filteredItems.Count == 0)
                {
                    if (_autocompleteType == 2)
                    {
                        filteredItems.Add(new CustomTextItem("No matching clients found", "Press Enter to add it as a client"));
                    }
                    else if (_autocompleteType == 3)
                    {
                        filteredItems.Add(new CustomTextItem("No matching projects", "Try a different keyword or create a new project"));
                    }
                    else if (_autocompleteType == 5)
                    {
                        filteredItems.Add(new CustomTextItem("No matching tags", "Press Enter to add it as a tag"));
                    }
                }

                VisibleItems = filteredItems;
            }
            LB.ItemsSource = VisibleItems;
            if (_autocompleteType == 3)
                this._selectionManager.SelectFirstItem();
        }

        private bool Filter(IAutoCompleteItem item)
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
        TAG = 5,

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
            {ItemType.TAG, "tag-item-template"},
            {ItemType.CLIENT, "client-item-template"},
            {ItemType.WORKSPACE, "workspace-item-template"},
            {ItemType.WORKSPACE_SEPARATOR, "workspace-separator-item-template"},
            {ItemType.CUSTOM_TEXT, "custom-text-item-template"},
        };
        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            return
                item is IAutoCompleteItem listItem && DataTemplateMap.TryGetValue(listItem.Type, out var resourceKey)
                    ? ((FrameworkElement) container).FindResource(resourceKey) as DataTemplate
                    : null;
        }
    }
}