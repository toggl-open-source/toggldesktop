using System;

namespace TogglDesktop.ViewModels
{
    public static class ViewModelExtensions
    {
        public static DayHeaderViewModel ToDayHeaderViewModel(this Toggl.TogglTimeEntryView item)
        {
            if (!item.IsHeader)
            {
                throw new InvalidOperationException("Can only create day header from header time entry view.");
            }

            return new DayHeaderViewModel(item.DateHeader, item.DateDuration);
        }
    }
}