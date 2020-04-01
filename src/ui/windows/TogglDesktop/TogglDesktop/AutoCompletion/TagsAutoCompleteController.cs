using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;
using TogglDesktop.AutoCompletion.Items;

namespace TogglDesktop.AutoCompletion
{
    class TagsAutoCompleteController : IAutoCompleteController
    {
        private readonly ListBoxSelectionManager<IAutoCompleteItem> _selectionManager = new ListBoxSelectionManager<IAutoCompleteItem>();
        private readonly IList<IAutoCompleteItem> _fullItemsList;
        private readonly Dictionary<string, TagItem> _tagItemsDictionary;
        private string _previousInput = string.Empty;
        private static readonly char[] _splitChars = { ' ' };
        public TagsAutoCompleteController(IEnumerable<string> list, Func<string, bool> isSelected)
        {
            _fullItemsList = list.Select(tag => (IAutoCompleteItem)new TagItem(tag))
                .AppendIfEmpty(() =>
                    new CustomTextItem("There are no tags yet",
                        "Start typing and press Enter to add a new tag"))
                .ToList();
            _tagItemsDictionary = _fullItemsList.OfType<TagItem>()
                .ToDictionary(tagItemViewModel => tagItemViewModel.Text, tagItemViewModel => tagItemViewModel);
            _tagItemsDictionary.ForEach(kvp => kvp.Value.IsChecked = isSelected(kvp.Key));
            VisibleItems = _fullItemsList;
        }

        public void AddTag(string tag)
        {
            if (_tagItemsDictionary.TryGetValue(tag, out var tagItemViewModel))
            {
                tagItemViewModel.IsChecked = true;
            }
            else
            {
                var newTag = new TagItem(tag) { IsChecked = true };
                AppendTag(newTag);
                VisibleItems = _fullItemsList;
            }
        }

        public void RemoveTag(string tag)
        {
            if (_tagItemsDictionary.TryGetValue(tag, out var tagItemViewModel))
            {
                tagItemViewModel.IsChecked = false;
            }
        }

        public void ClearSelection()
        {
            _tagItemsDictionary.Values.ForEach(x => x.IsChecked = false);
        }

        public string DebugIdentifier => "Tags";

        private ListBox ListBox
        {
            get => _selectionManager.ListBox;
            set => _selectionManager.ListBox = value;
        }

        public IAutoCompleteItem SelectedItem =>
            (ListBox != null
             && ListBox.SelectedIndex != -1)
             ? VisibleItems[ListBox.SelectedIndex]
             : null;

        public IList<IAutoCompleteItem> VisibleItems
        {
            get => _selectionManager.Items;
            private set
            {
                _selectionManager.Items = value;
                if (ListBox != null) ListBox.ItemsSource = value;
            }
        }

        public bool ShowActionButton { get; private set; }

        public void FillList(ListBox listBox)
        {
            ListBox = listBox;
            VisibleItems = _fullItemsList;
            ShowActionButton = false;
        }

        public void Complete(string input)
        {
            if (string.IsNullOrWhiteSpace(input))
            {
                VisibleItems = _fullItemsList;
                ShowActionButton = false;
                return;
            }

            var filterWords = input.Split(_splitChars, StringSplitOptions.RemoveEmptyEntries);
            var itemsToFilter =
                (_previousInput != null && !input.StartsWith(_previousInput)
                    ? _fullItemsList
                    : VisibleItems);
            _previousInput = input;
            VisibleItems = itemsToFilter.Where(item =>
                    filterWords.All(word => item.Text.IndexOf(word, StringComparison.OrdinalIgnoreCase) != -1))
                .AppendIfEmpty(() => new CustomTextItem("No matching tags", "Press Enter to add it as a tag"))
                .ToList();

            ShowActionButton = !VisibleItems.Any(it => it.Text == input && it.Type == ItemType.TAG);
        }

        public void SelectNext()
        {
            _selectionManager.SelectNext();
        }

        public void SelectPrevious()
        {
            _selectionManager.SelectPrevious();
        }

        private void AppendTag(TagItem tagItem)
        {
            if (_fullItemsList[0].Type == ItemType.CUSTOM_TEXT)
            {
                _fullItemsList.Clear();
            }

            _fullItemsList.Add(tagItem);
            _tagItemsDictionary[tagItem.Text] = tagItem;
        }
    }
}