using System.Collections.Generic;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    interface IAutoCompleteController
    {
        string DebugIdentifier { get; }
        AutoCompleteItem SelectedItem { get; }
        IList<ListBoxItemViewModel> VisibleItems { get; }
        bool ShowActionButton { get; }
        void FillList(ListBox listBox);
        void Complete(string input);
        void SelectNext();
        void SelectPrevious();
    }
}