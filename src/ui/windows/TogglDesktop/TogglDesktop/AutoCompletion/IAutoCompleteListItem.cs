using System;
using System.Collections.Generic;
using System.Windows.Controls;
using TogglDesktop.AutoCompleteControls;

namespace TogglDesktop.AutoCompletion
{
    interface IAutoCompleteListItem
    {
        string Text { get; }
        bool Visible { get; }

        IEnumerable<AutoCompleteItem> Complete(string[] words);
        IEnumerable<AutoCompleteItem> CompleteAll();
        IEnumerable<AutoCompleteItem> CompleteVisible();

        void CreateFrameworkElement(
            Panel parent, Action<AutoCompleteItem> selectWithClick,
            List<IRecyclable> recyclables, AutoCompleteController controller
            );
    }
}