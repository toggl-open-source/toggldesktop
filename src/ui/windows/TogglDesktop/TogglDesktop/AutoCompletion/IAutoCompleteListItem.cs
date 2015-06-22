using System;
using System.Collections.Generic;
using System.Windows.Controls;

namespace TogglDesktop.AutoCompletion
{
    interface IAutoCompleteListItem
    {
        string Text { get; }
        bool Visible { get; }
        IEnumerable<AutoCompleteItem> Complete(string input);
        IEnumerable<AutoCompleteItem> CompleteAll();
        void CreateFrameworkElement(Panel parent, Action<AutoCompleteItem> selectWithClick);
    }
}