using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive;
using System.Windows;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion.Items;
using TogglDesktop.Diagnostics;
using TogglDesktop.ViewModels;

namespace TogglDesktop.ui.ViewModels
{
    public class TimerViewModel : ReactiveObject
    {
        private readonly DispatcherTimer secondsTimer = new DispatcherTimer();
        private Toggl.TogglTimeEntryView runningTimeEntry;
        private bool acceptNextUpdate;
        private bool isMiniTimer;
        private Toggl.TogglAutocompleteView completedProject;

        public TimerViewModel(bool isMiniTimer)
        {
            this.isMiniTimer = isMiniTimer;
            StartStopCommand = ReactiveCommand.Create(() => startStop());
            CancelProjectSelectionCommand = ReactiveCommand.Create(() => clearSelectedProject());
            ManualAddButtonCommand = ReactiveCommand.Create(() => AddNewTimeEntry());
            DescriptionAutoCompleteConfirmCommand = ReactiveCommand.Create<RoutedEventArgs>(e => {
                DescriptionAutoCompleteConfirm(e);
            });
            TryOpenEditView = ReactiveCommand.Create<string>(s => tryOpenEditViewIfRunning(s));

            setupSecondsTimer();

            Toggl.OnRunningTimerState += onRunningTimerState;
            Toggl.OnStoppedTimerState += onStoppedTimerState;

            ResetRunningTimeEntry(false, true);
        }

        [Reactive]
        public bool IsRunning { get; set; }

        [Reactive]
        public string DurationText { get; set; }

        [Reactive]
        public string Description { get; set; }

        [Reactive]
        public string DurationPanelToolTip { get; private set; }

        [Reactive]
        public TimeEntryLabelViewModel TimeEntryLabelViewModel { get; private set; }

        [Reactive]
        public ProjectLabelViewModel ProjectLabelViewModel { get; private set; }

        public ReactiveCommand<Unit, Unit> StartStopCommand { get; }

        public ReactiveCommand<Unit, Unit> CancelProjectSelectionCommand { get; }

        public ReactiveCommand<Unit, Unit> ManualAddButtonCommand { get; }

        public ReactiveCommand<RoutedEventArgs, Unit> DescriptionAutoCompleteConfirmCommand { get; }

        public ReactiveCommand<string, Unit> TryOpenEditView { get; }

        private void setupSecondsTimer()
        {
            secondsTimer.Interval = TimeSpan.FromSeconds(1);
            secondsTimer.Tick += (sender, args) =>
            {
                if (!IsRunning)
                    return;

                DurationText = Toggl.FormatDurationInSecondsHHMMSS(runningTimeEntry.DurationInSeconds);
            };
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            using (Performance.Measure("timer responding to OnRunningTimerState"))
            {
                
                SetRunningTimeEntry(te);
                secondsTimer.IsEnabled = true;
            }
        }

        private void SetRunningTimeEntry(Toggl.TogglTimeEntryView item)
        {
            ResetRunningTimeEntry(true);
            runningTimeEntry = item;
            TimeEntryLabelViewModel = item.ToTimeEntryLabelViewModel();
            DurationText = Toggl.FormatDurationInSecondsHHMMSS(item.DurationInSeconds);
            DurationPanelToolTip = "started at " + item.StartTimeString;
        }

        private void ResetRunningTimeEntry(bool running, bool forceUpdate = false)
        {
            var changedState = IsRunning != running;

            if (!(changedState || forceUpdate || this.acceptNextUpdate))
                return;

            acceptNextUpdate = false;

            IsRunning = running;
            Description = "";
            DurationText = "";
        }

        private void onStoppedTimerState()
        {
            using (Performance.Measure("timer responding to OnStoppedTimerState"))
            {
                secondsTimer.IsEnabled = false;
                ResetRunningTimeEntry(false);
                runningTimeEntry = default(Toggl.TogglTimeEntryView);
            }
        }

        #region controlling

        public void startStop()
        {
            this.acceptNextUpdate = true;

            if (IsRunning)
            {
                this.stop();
            }
            else
            {
                this.start();
            }
        }

        public void tryOpenEditViewIfRunning(string focusedField)
        {
            if (IsRunning)
            {
                using (Performance.Measure("opening edit view from timer, focussing " + focusedField))
                {
                    Toggl.Edit(this.runningTimeEntry.GUID, false, focusedField);
                }
            }
        }

        private void start()
        {
            using (Performance.Measure("starting time entry from timer"))
            {
                var guid = Toggl.Start(
                    Description,
                    "",
                    completedProject.TaskID,
                    completedProject.ProjectID,
                    "",
                    completedProject.Tags,
                    isMiniTimer
                    );

                if (completedProject.Billable)
                {
                    Toggl.SetTimeEntryBillable(guid, true);
                }

                this.clearSelectedProject();
            }
        }

        private void stop()
        {
            using (Performance.Measure("stopping time entry from timer"))
            {
                Toggl.Stop(isMiniTimer);
            }
        }

        #endregion

        public void clearSelectedProject()
        {
            ProjectLabelViewModel = null;
            completedProject = default;
        }

        public void DescriptionAutoCompleteConfirm(object e)
        {
            var asItem = e as IModelItem<Toggl.TogglAutocompleteView>;
            if (asItem == null)
                return;

            var item = asItem.Model;

            Description = item.Description;

            ProjectLabelViewModel = item.ToProjectLabelViewModel();
            completedProject = item;
        }

        private void AddNewTimeEntry()
        {
            var guid = Toggl.Start("", "0", 0, 0, "", "", isMiniTimer);
            Toggl.Edit(guid, false, Toggl.Duration);
        }
    }
}
