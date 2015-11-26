
using System;

namespace TogglDesktop.AutoCompletion
{
    interface ICollapsable
    {
        event EventHandler CollapsedChanged;
        bool Collapsed { get; set; }
    }
}