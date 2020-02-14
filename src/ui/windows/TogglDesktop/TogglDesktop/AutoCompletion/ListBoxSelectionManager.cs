using System;
using System.Collections.Generic;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    public class ListBoxSelectionManager<T> where T: ISelectable
    {
        private int selectedIndex;
        public IList<T> Items { get; set; }
        public ListBox ListBox { get; set; }

        public void SelectNext()
        {
            SelectItem(index => index + 1);
        }

        public void SelectPrevious()
        {
            SelectItem(index => index - 1);
        }

        private void SelectItem(Func<int, int> nextIndexSelector)
        {
            if (this.Items == null || this.Items.Count == 0) return;
            var maxIterations = Items.Count;
            var initialIndex = this.selectedIndex % Items.Count;
            var nextIndex = initialIndex;
            for (var i = 0; i < maxIterations; i++)
            {
                nextIndex = (nextIndexSelector(nextIndex) + this.Items.Count) % this.Items.Count;
                if (Items[nextIndex].IsSelectable)
                {
                    this.selectIndex(nextIndex);
                    return;
                }
            }

            if (Items[initialIndex].IsSelectable)
            {
                this.selectIndex(initialIndex);
            }
        }

        public void SelectFirstItem()
        {
            if (Items.Count == 0) return;

            if (Items[0].IsSelectable)
            {
                selectIndex(0);
            }
            else
            {
                this.selectedIndex = 0;
                SelectNext();
            }
        }

        private void selectIndex(int index)
        {
            if (index < 0 || this.Items.Count == 0 || index >= this.Items.Count)
                index = 0;

            this.selectedIndex = index;
            ListBox.SelectedIndex = index;

            ListBox.UpdateLayout();
            if (this.Items.Count > 0 && ListBox.SelectedIndex != -1)
                ListBox.ScrollIntoView(ListBox.Items[ListBox.SelectedIndex]);
        }

    }
}