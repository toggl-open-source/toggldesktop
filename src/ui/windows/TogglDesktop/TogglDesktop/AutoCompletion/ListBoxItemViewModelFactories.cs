using System.Collections.Generic;
using System.Linq;

namespace TogglDesktop.AutoCompletion
{
    public static class ListBoxItemViewModelFactories
    {
        internal static List<ListBoxItemViewModel> ToTagItemViewModelsList(this IEnumerable<string> tagsList)
        {
            return tagsList.ToTagItemViewModels().ToList();
        }

        internal static IEnumerable<ListBoxItemViewModel> ToTagItemViewModels(this IEnumerable<string> tagsList)
        {
            return tagsList
                .Select(tag =>
                {
                    var tagItemViewModel = new TagItemViewModel(tag);
                    return (ListBoxItemViewModel) tagItemViewModel;
                })
                .AppendIfEmpty(() =>
                    new CustomTextItemViewModel("There are no tags yet",
                        "Start typing and press Enter to add a new tag"));
        }
    }
}