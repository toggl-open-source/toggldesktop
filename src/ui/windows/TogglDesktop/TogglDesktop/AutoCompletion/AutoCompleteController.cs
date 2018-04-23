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

        private string[] categories = { "Time Entries", "Tasks", "Projects", "Workspaces" };

        private readonly List<IAutoCompleteListItem> list;
        private List<ListBoxItem> items;
        public List<ListBoxItem> visibleItems;
        private ListBox LB;
        public string DebugIdentifier { get; private set; }

        private int selectedIndex;
        private string filterText;
        private string[] words;

        public AutoCompleteController(List<IAutoCompleteListItem> list, string debugIdentifier)
        {
            this.list = list;
            this.DebugIdentifier = debugIdentifier;
        }

        public AutoCompleteItem SelectedItem
        {
            get { return (LB.SelectedIndex != -1) ? (AutoCompleteItem)this.list[visibleItems[LB.SelectedIndex].Index] : null; }
        }

        public List<IAutoCompleteListItem> getList()
        {
            return this.list;
        }

        public AutoCompleteItem getItemByRow(ListBoxItem listitem)
        {
            return (AutoCompleteItem)this.list[listitem.Index];
        }

        public void FillList(ListBox listBox, Action<AutoCompleteItem> selectWithClick, List<IRecyclable> recyclables)
        {
            LB = listBox;
            int lastType = -1;
            using (Performance.Measure("FILLIST, {0} items", this.list.Count))
            {
                items = new List<ListBoxItem>();
                for (var count = 0; count < this.list.Count; ++count)
                {
                    var item = this.list[count];
                    var it = (TimerItem)item;

                    // Add category title if needed
                    if (lastType != (int)it.Item.Type) {
                        items.Add(new ListBoxItem() {
                            Category = categories[(int)it.Item.Type],
                            Type = -1
                        });
                    }

                    items.Add(new ListBoxItem() {
                        Text = it.Item.Text,
                        Description = it.Item.Description,
                        ProjectLabel = it.Item.ProjectLabel,
                        ProjectColor = it.Item.ProjectColor,
                        TaskLabel = it.Item.TaskLabel,
                        ClientLabel = it.Item.ClientLabel,
                        Type = (int)it.Item.Type,
                        Index = count
                    });
                    lastType = (int)it.Item.Type;
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
                visibleItems = visibleItems.Where(i => Filter(i)).ToList();
            }
            LB.ItemsSource = visibleItems;
        }

        private bool Filter(object item)
        {
            if (String.IsNullOrEmpty(filterText))
                return true;

            var listItem = (ListBoxItem)item;

            if (listItem.Type == -1)
                return true;

            foreach (string word in words)
            {
                if (listItem.Text.IndexOf(word, StringComparison.OrdinalIgnoreCase) >= 0) 
                {
                    return true;
                }
            }
            return false;
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

        private void selectIndex(int index)
        {
            if (index < -1 || index >= this.visibleItems.Count)
                throw new ArgumentOutOfRangeException("index");

            this.selectedIndex = index;
            LB.SelectedIndex = index;

            LB.UpdateLayout();
            LB.ScrollIntoView(LB.Items[LB.SelectedIndex]);
        }

        public void SelectNext()
        {
            if (this.visibleItems.Count == 0)
                return;

            var i = this.selectedIndex + 1;
            if (i == this.visibleItems.Count)
                i = 0;
            this.selectIndex(i);
        }

        public void SelectPrevious()
        {
            if (this.visibleItems.Count == 0)
                return;

            var i = this.selectedIndex - 1;
            if (i < 0)
                i = this.visibleItems.Count - 1;
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
        private const int TIMEENTRY = 0;
        private const int TASK = 1;
        private const int PROJECT = 2;
        private const int WORKSPACE = 3;

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

            return null;
        }
    }
}