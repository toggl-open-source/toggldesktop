using System.Reactive.Linq;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;

namespace TogglDesktop.ViewModels
{
    public class TimeEntryListViewModel : ReactiveObject
    {
        public TimeEntryListViewModel()
        {
            this.WhenAnyValue(x => x.IsLoading)
                .Select(x => !x)
                .ToPropertyEx(this, x => x.IsViewEnabled);
        }

        [Reactive]
        public bool ShowLoadMore { get; private set; }

        [Reactive]
        public bool IsLoading { get; private set; }

        public bool IsViewEnabled { [ObservableAsProperty] get; }

        [Reactive]
        public Screen CurrentScreen { get; private set; }

        public void OnTimeEntryList(bool showLoadMore, bool isListEmpty)
        {
            IsLoading = false;
            ShowLoadMore = showLoadMore;
            CurrentScreen = GetCurrentScreen(showLoadMore, isListEmpty);
        }

        public void LoadMore()
        {
            IsLoading = true;
            Toggl.LoadMore();
        }

        private static Screen GetCurrentScreen(bool showLoadMore, bool isListEmpty) =>
            isListEmpty ?
                (showLoadMore ?
                    Screen.NoRecentEntries
                    : Screen.Welcome)
                : Screen.TimeEntriesList;
    }

    public enum Screen
    {
        TimeEntriesList,
        Welcome,
        NoRecentEntries
    }
}