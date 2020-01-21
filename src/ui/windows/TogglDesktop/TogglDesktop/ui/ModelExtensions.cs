namespace TogglDesktop
{
    public static class ModelExtensions
    {
        public static string GetGUID(this Toggl.TogglTimeEntryView item)
        {
            return item.GUID + (item.Group ? "true" : "");
        }

        public static string GetGUID(this TimeEntryCell cell)
        {
            return cell.GUID + (cell.IsGroup ? "true" : "");
        }
    }
}