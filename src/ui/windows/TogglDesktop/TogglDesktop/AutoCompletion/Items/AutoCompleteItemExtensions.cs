namespace TogglDesktop.AutoCompletion.Items
{
    public static class AutoCompleteItemExtensions
    {
        public static bool IsSelectable(this IAutoCompleteItem item) => (int) item.Type >= 0;
    }
}