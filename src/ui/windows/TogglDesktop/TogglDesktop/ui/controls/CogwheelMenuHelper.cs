using System.Windows;
using System.Windows.Controls;

namespace TogglDesktop
{
    public static class CogwheelMenuHelper
    {
        public static void Initialize(this ContextMenu menu, IInputElement target)
        {
            foreach (var item in menu.Items)
            {
                var asMenuItem = item as MenuItem;
                if (asMenuItem != null)
                {
                    asMenuItem.CommandTarget = target;
                }
            }
        }
    }
}
