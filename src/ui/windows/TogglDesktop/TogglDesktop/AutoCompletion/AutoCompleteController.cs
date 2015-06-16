using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    class AutoCompleteController
    {
        private readonly Dictionary<string, AutoCompleteItem> elementByText;
        private readonly Dictionary<AutoCompleteItem, string> textByElement;
        private readonly List<IAutoCompleteListItem> list;

        private readonly List<AutoCompleteItem> currentlyVisible = new List<AutoCompleteItem>();
        private readonly ReadOnlyCollection<AutoCompleteItem> currentlyVisibleAsReadonly;

        private int selectedIndex;
        private AutoCompleteItem selectedItem;

        public AutoCompleteController(List<IAutoCompleteListItem> list, IReadOnlyCollection<AutoCompleteItem> items)
        {
            this.list = list;

            this.elementByText = new Dictionary<string, AutoCompleteItem>(items.Count);
            this.textByElement = new Dictionary<AutoCompleteItem, string>(items.Count);

            foreach (var item in items)
            {
                var text = item.Text;
                this.elementByText.Add(text, item);
                this.textByElement.Add(item, text);
            }

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

        public void FillList(Panel list)
        {
            foreach (var item in this.list)
            {
                item.CreateFrameworkElement(list);
            }
        }

        public void Complete(string input)
        {
            this.currentlyVisible.Clear();
            this.currentlyVisible.AddRange(this.list.SelectMany(i => i.Complete(input)));
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
    }
}