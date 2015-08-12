using System.Windows.Forms;

namespace TogglDesktop
{
class ComboBoxHelper
{
    public static ulong SelectedItemID(ComboBox combobox)
    {
        for (int i = 0; i < combobox.Items.Count; i++)
        {
            Toggl.TogglGenericView item = (Toggl.TogglGenericView)combobox.Items[i];
            if (item.Name == combobox.Text)
            {
                return item.ID;
            }
        }
        return 0;
    }
}
}
