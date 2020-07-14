using System.Collections.Generic;
using System.Windows.Controls;
using TogglTrack.AutoCompletion.Items;

namespace TogglTrack.AutoCompletion
{
    interface IAutoCompleteController
    {
        string DebugIdentifier { get; }
        IAutoCompleteItem SelectedItem { get; }
        IList<IAutoCompleteItem> VisibleItems { get; }
        bool ShowActionButton { get; }
        void FillList(ListBox listBox);
        void Complete(string input);
        void SelectNext();
        void SelectPrevious();
    }
}