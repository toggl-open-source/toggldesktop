using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;
using TogglDesktop.Diagnostics;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class TimerEntryListView : IMainView
    {
        public TimerEntryListView()
        {
            this.InitializeComponent();

            Toggl.OnTimeEntryEditor.Select(x => x.timeEntry.GUID)
                .ObserveOnDispatcher()
                .Subscribe(this.Entries.SelectEntry);
            Toggl.OnTimeEntryList.ObserveOnDispatcher().Subscribe(this.onTimeEntryList);
            Toggl.OnLogin.Where(x => x.open || x.userId == 0).ObserveOnDispatcher()
                .Subscribe(_ => this.Entries.Children.Clear());
        }

        public Brush TitleBarBrush => this.Timer.Background;
        public double TimerHeight => this.Timer.Height;

        protected override void OnInitialized(EventArgs e)
        {
            // need to use code behind because binding to child elements won't work
            // TODO: find a way to express this in XAML
            this.MinHeight = this.Timer.Height;
            this.MinWidth = this.Timer.MinWidth;

            base.OnInitialized(e);
        }

        #region toggl events

        private void onTimeEntryList((bool open, List<Toggl.TogglTimeEntryView> list, bool showLoadMoreButton) x)
        {
            var (open, list, showLoadMoreButton) = x;

            this.fillTimeEntryList(list);
            this.Entries.ViewModel.OnTimeEntryList(showLoadMoreButton, list.Count == 0);
        }

        #endregion

        private void fillTimeEntryList(List<Toggl.TogglTimeEntryView> list)
        {
            var previousCount = this.Entries.EntriesCount;
            var newCount = list.Count;

            using (Performance.Measure("rendering time entry list, previous count: {0}, new count: {1}", previousCount, newCount))
            {
                var days = groupByDays(list);
                var dayHeaderViewModels = this.fillDays(days);

                this.Entries.SetDayHeaderViewModels(dayHeaderViewModels);
            }
        }

        private DayHeaderViewModel[] fillDays(List<List<Toggl.TogglTimeEntryView>> days)
        {
            var children = this.Entries.Children;

            // remember which days were expanded
            var isExpandedDictionary = children
                .Cast<TimeEntryCellDayHeader>()
                .Select(h => h.ViewModel)
                .ToDictionary(vm => vm.DateHeader, vm => vm.IsExpanded);

            // remove superfluous days
            if (children.Count > days.Count)
            {
                var daysToRemoveCount = children.Count - days.Count;
                children.RemoveRange(days.Count, daysToRemoveCount);
            }
            else
            {
                var daysToAddCount = days.Count - children.Count;
                for (var i = 0; i < daysToAddCount; i++)
                    children.Add(new TimeEntryCellDayHeader());
            }

            var viewModels = days.Select(day =>
            {
                var vm = day[0].ToDayHeaderViewModel();
                var isExpanded = isExpandedDictionary.GetValueOrDefault(day[0].DateHeader, true);
                vm.IsExpanded = isExpanded;
                return vm;
            }).ToArray();

            for (var i = 0; i < children.Count; i++)
            {
                ((TimeEntryCellDayHeader)children[i]).Display(viewModels[i], days[i]);
            }

            return viewModels;
        }

        private static List<List<Toggl.TogglTimeEntryView>> groupByDays(List<Toggl.TogglTimeEntryView> list)
        {
            var days = new List<List<Toggl.TogglTimeEntryView>>();
            List<Toggl.TogglTimeEntryView> currentDay = null;

            foreach (var item in list)
            {
                if (item.IsHeader)
                {
                    currentDay = new List<Toggl.TogglTimeEntryView>();
                    days.Add(currentDay);
                }

                currentDay.Add(item);
            }

            return days;
        }

        public void SetListWidth(double width)
        {
            this.Entries.HorizontalAlignment = HorizontalAlignment.Left;
            this.Entries.Width = Math.Max(width, 0);
        }

        public void DisableListWidth()
        {
            this.Entries.HorizontalAlignment = HorizontalAlignment.Stretch;
            this.Entries.Width = this.Width;
        }

        public void SetManualMode(bool manualMode)
        {
            this.Timer.SetManualMode(manualMode);
        }

        private void onFocusTimeEntryList(object sender, EventArgs e)
        {
            this.Entries.Focus(true);
        }
        private void onFocusTimer(object sender, EventArgs e)
        {
            this.Timer.Focus();
        }

        public void Activate(bool allowAnimation)
        {
            this.IsEnabled = true;
            var focusTimerAction = new Action(() => { this.Timer.Focus(); });
            this.Dispatcher.BeginInvoke(DispatcherPriority.SystemIdle, focusTimerAction);
        }

        public void Deactivate(bool allowAnimation)
        {
            this.IsEnabled = false;
        }

        public bool HandlesError(string errorMessage)
        {
            return false;
        }
    }
}
