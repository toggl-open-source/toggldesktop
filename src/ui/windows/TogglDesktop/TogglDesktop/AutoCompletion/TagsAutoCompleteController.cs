using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    class TagsAutoCompleteController : IAutoCompleteController
    {
        private readonly ListBoxSelectionManager<ListBoxItemViewModel> _selectionManager = new ListBoxSelectionManager<ListBoxItemViewModel>();
        private readonly IList<ListBoxItemViewModel> _fullItemsList;
        private readonly Dictionary<string, TagItemViewModel> _tagItemsDictionary;
        private string _previousInput = string.Empty;
        private static readonly char[] _splitChars = { ' ' };
        public TagsAutoCompleteController(IEnumerable<string> list, Func<string, bool> isSelected)
        {
            _fullItemsList = list.Select(tag => (ListBoxItemViewModel)new TagItemViewModel(tag))
                .AppendIfEmpty(() =>
                    new CustomTextItemViewModel("There are no tags yet",
                        "Start typing and press Enter to add a new tag"))
                .ToList();
            _tagItemsDictionary = _fullItemsList.OfType<TagItemViewModel>()
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
        }

        public void RemoveTag(string tag)
        {
            if (_tagItemsDictionary.TryGetValue(tag, out var tagItemViewModel))
            {
                tagItemViewModel.IsChecked = false;
            }
        }

        public string DebugIdentifier => "Tags";

        private ListBox ListBox
        {
            get => _selectionManager.ListBox;
            set => _selectionManager.ListBox = value;
        }

        public AutoCompleteItem SelectedItem =>
            (ListBox != null
             && ListBox.SelectedIndex != -1
             && VisibleItems[ListBox.SelectedIndex] is IModelItemViewModel modelItem)
                ? modelItem.Model
                : null;

        public IList<ListBoxItemViewModel> VisibleItems
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
                .AppendIfEmpty(() => new CustomTextItemViewModel("No matching tags", "Press Enter to add it as a tag"))
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
    }
}