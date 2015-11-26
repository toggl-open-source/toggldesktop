using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion
{
    class AutoCompleteController
    {
        private static readonly char[] splitChars = { ' ' };

        private readonly List<IAutoCompleteListItem> list;
        public string DebugIdentifier { get; private set; }

        private readonly List<AutoCompleteItem> currentlyVisible = new List<AutoCompleteItem>();
        private readonly ReadOnlyCollection<AutoCompleteItem> currentlyVisibleAsReadonly;

        private int selectedIndex;
        private AutoCompleteItem selectedItem;

        public AutoCompleteController(List<IAutoCompleteListItem> list, string debugIdentifier)
        {
            this.list = list;
            this.DebugIdentifier = debugIdentifier;

            this.currentlyVisibleAsReadonly = this.currentlyVisible.AsReadOnly();
        }

        public AutoCompleteItem SelectedItem
        {
            get { return this.selectedItem; }
        }

        public ReadOnlyCollection<AutoCompleteItem> VisibleItems
        {
            get { return this.currentlyVisibleAsReadonly; }
        }

        public void FillList(Panel list, Action<AutoCompleteItem> selectWithClick, List<IRecyclable> recyclables)
        {
            foreach (var item in this.list)
            {
                item.CreateFrameworkElement(list, selectWithClick, recyclables, this);
            }
        }

        public void Complete(string input)
        {
            var words = input.Split(splitChars, StringSplitOptions.RemoveEmptyEntries);

            this.completeWith(i => i.Complete(words));
        }

        public void RefreshVisibleList()
        {
            this.completeWith(i => i.CompleteVisible());
        }

        private void completeWith(Func<IAutoCompleteListItem, IEnumerable<AutoCompleteItem>> completor)
        {
            this.currentlyVisible.Clear();
            this.currentlyVisible.AddRange(this.list.SelectMany(completor));
            this.validateSelection();
        }

        private void validateSelection()
        {
            if (this.selectedIndex == -1)
                return;
            var selectedItemIndex = this.currentlyVisible.IndexOf(this.selectedItem);
            this.selectIndex(selectedItemIndex);
            if (this.selectedIndex != -1)
                this.SelectedItem.Selected = true;
        }

        private void selectIndex(int index)
        {
            if(index < -1 || index >= this.currentlyVisible.Count)
                throw new ArgumentOutOfRangeException("index");

            this.selectedIndex = index;
            var newItem = index == -1 ? null : this.currentlyVisible[index];
            if (newItem == this.selectedItem)
                return;

            if (this.selectedItem != null)
                this.selectedItem.Selected = false;

            this.selectedItem = newItem;

            if (this.selectedItem != null)
                this.selectedItem.Selected = true;
        }

        public void SelectNext()
        {
            if (this.currentlyVisible.Count == 0)
                return;

            var i = this.selectedIndex + 1;
            if (i == this.currentlyVisible.Count)
                i = 0;
            this.selectIndex(i);
        }

        public void SelectPrevious()
        {
            if (this.currentlyVisible.Count == 0)
                return;

            var i = this.selectedIndex - 1;
            if (i < 0)
                i = this.currentlyVisible.Count - 1;
            this.selectIndex(i);
        }

        public void SelectItem(AutoCompleteItem item)
        {
            var i = item == null ? -1 : this.currentlyVisible.IndexOf(item);
            this.selectIndex(i);
        }

        public bool TryCollapseCategory()
        {
            var asItemCategory = this.selectedItem as AutoCompleteItemCategory;
            if (asItemCategory == null || asItemCategory.Collapsed)
                return false;

            asItemCategory.Collapsed = true;
            this.RefreshVisibleList();
            return true;
        }

        public bool TryExpandCategory()
        {
            var asItemCategory = this.selectedItem as AutoCompleteItemCategory;
            if (asItemCategory == null || !asItemCategory.Collapsed)
                return false;

            asItemCategory.Collapsed = false;
            this.RefreshVisibleList();
            return true;
        }
    }
}