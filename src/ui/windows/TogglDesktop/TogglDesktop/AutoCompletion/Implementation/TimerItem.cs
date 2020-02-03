namespace TogglDesktop.AutoCompletion.Implementation
{
    sealed class TimerItem : SimpleItem<Toggl.TogglAutocompleteView>
    {
        public TimerItem(Toggl.TogglAutocompleteView item, bool isProject)
            : this(item, isProject ? item.ProjectAndTaskLabel : item.Description)
        {
        }

        private TimerItem(Toggl.TogglAutocompleteView item, string text)
            : base(item, text)
        {
        }
    }
}