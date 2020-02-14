using System.Collections.Generic;

namespace TogglDesktop.AutoCompletion
{
    class TagsAutoCompleteController : AutoCompleteController
    {
        public TagsAutoCompleteController(List<IAutoCompleteListItem> list, string debugIdentifier)
            : base(list, debugIdentifier, 5)
        {
        }

        // public void AddTag(string tag)
        // {
        //     if (autocompleteType != 5) throw new InvalidOperationException(nameof(AddTag) + "called on a non-tag controller");
        //     if (_tagsDictionary == null) throw new InvalidOperationException(nameof(AddTag) + "called on a non-initialized controller");
        //     if (_tagsDictionary.TryGetValue(tag, out var tagItemViewModel))
        //     {
        //         tagItemViewModel.IsChecked = true;
        //     }
        // }

        // public void RemoveTag(string tag)
        // {
        //     if (autocompleteType != 5) throw new InvalidOperationException(nameof(RemoveTag) + "called on a non-tag controller");
        //     if (_tagsDictionary == null) throw new InvalidOperationException(nameof(RemoveTag) + "called on a non-initialized controller");
        //     if (_tagsDictionary.TryGetValue(tag, out var tagItemViewModel))
        //     {
        //         tagItemViewModel.IsChecked = false;
        //     }
        // }
    }
}