using System;
using System.Windows.Controls;

namespace TogglDesktop.WPF
{
    static class Extensions
    {
        public static bool TryInvoke(this Control control, Action action)
        {
            if(control.Dispatcher.CheckAccess())
                return false;

            control.Dispatcher.Invoke(action);

            return true;
        }
    }
}
