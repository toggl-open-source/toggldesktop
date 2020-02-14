using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    class TagsAutoCompleteController : IAutoCompleteController
    {
        private readonly ListBoxSelectionManager<ListBoxItemViewModel> _selectionManager = new ListBoxSelectionManager<ListBoxItemViewModel>();
        private IList<ListBoxItemViewModel> _fullItemsList;
        private Dictionary<string, TagItemViewModel> _tagItemsDictionary;
        private string _previousInput = string.Empty;
        private static readonly char[] _splitChars = { ' ' };
        public TagsAutoCompleteController(IEnumerable<string> list, Func<string, bool> isSelected)
        {
            _fullItemsList = list.ToTagItemViewModelsList();
            _tagItemsDictionary = _fullItemsList.OfType<TagItemViewModel>()
                .ToDictionary(tagItemViewModel => tagItemViewModel.Text, tagItemViewModel => tagItemViewModel);
            _tagItemsDictionary.ForEach(kvp => kvp.Value.IsChecked = isSelected(kvp.Key));
            VisibleItems = _fullItemsList;
        }

        public void UpdateWith(IEnumerable<string> list, Func<string, bool> isSelected)
        {
            var oldItemsDictionary = _tagItemsDictionary;
            _fullItemsList = list
                .Select(tag =>
                    oldItemsDictionary.TryGetValue(tag, out var tagItemViewModel) ? tagItemViewModel : new TagItemViewModel(tag))
                .Cast<ListBoxItemViewModel>()
                .ToList();
            _tagItemsDictionary = _fullItemsList.OfType<TagItemViewModel>()
                .ToDictionary(tagItemViewModel => tagItemViewModel.Text, tagItemViewModel => tagItemViewModel);
            _tagItemsDictionary.ForEach(kvp => kvp.Value.IsChecked = isSelected(kvp.Key));
            if (ListBox != null)
            {
                FillList(ListBox);
            }
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

        public void FillList(ListBox listBox)
        {
            ListBox = listBox;
            VisibleItems = _fullItemsList;
        }

        public void Complete(string input)
        {
            if (string.IsNullOrWhiteSpace(input))
            {
                VisibleItems = _fullItemsList;
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

            this._selectionManager.SelectFirstItem();
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