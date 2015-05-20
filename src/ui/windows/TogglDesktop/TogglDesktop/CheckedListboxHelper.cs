using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace TogglDesktop
{
    class CheckedListBoxHelper
    {
        public static List<String> CheckedStrings(CheckedListBox checkedListBox)
        {
            List<String> result = new List<String>();
            foreach (object item in checkedListBox.CheckedItems)
            {
                result.Add(item.ToString());
            }
            return result;
        }
    }
}
