using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;
using TogglDesktop.ViewModels;

namespace TogglDesktop
{
    public partial class MiniTimer
    {
        private readonly DispatcherTimer secondsTimer = new DispatcherTimer();
        private Toggl.TogglTimeEntryView runningTimeEntry;
        private bool isRunning;
        private bool acceptNextUpdate;
        private Toggl.TogglAutocompleteView completedProject;

        public MiniTimer()
        {
            this.InitializeComponent();

            this.setupSecondsTimer();

            Toggl.OnMinitimerAutocomplete += this.onMiniTimerAutocomplete;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;

            this.resetUIState(false, true);
        }

        public event MouseButtonEventHandler MouseCaptured;

        private static bool IsMiniTimer => true;

        private void setupSecondsTimer()
        {
            this.secondsTimer.Interval = TimeSpan.FromSeconds(1);
            this.secondsTimer.Tick += (sender, args) =>
            {
                if (!this.isRunning)
                    return;

                var s = Toggl.FormatDurationInSecondsHHMMSS(this.runningTimeEntry.DurationInSeconds);
                durationLabel.Text = s;
            };
        }

        #region toggl events

        private void onStoppedTimerState()
        {
            if (this.TryBeginInvoke(this.onStoppedTimerState))
                return;

            using (Performance.Measure("timer responding to OnStoppedTimerState"))
            {
                this.secondsTimer.IsEnabled = false;
                this.resetUIState(false);
                this.runningTimeEntry = default(Toggl.TogglTimeEntryView);
            }
        }

        private void onRunningTimerState(Toggl.TogglTimeEntryView te)
        {
            if (this.TryBeginInvoke(this.onRunningTimerState, te))
                return;

            using (Performance.Measure("timer responding to OnRunningTimerState"))
            {
                this.runningTimeEntry = te;
                this.setUIToRunningState(te);
                this.secondsTimer.IsEnabled = true;
            }
        }

        private void onMiniTimerAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onMiniTimerAutocomplete, list))
                return;

            using (Performance.Measure("timer building auto complete controller, {0} items", list.Count))
            {
                this.descriptionAutoComplete.SetController(AutoCompleteControllers.ForTimer(list));
            }
        }

        #endregion

        #region ui events

        private void startStopButtonOnClick(object sender, RoutedEventArgs e)
        {
            this.startStop();
        }

        private void onGridKeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Enter:
                {
                    this.startStopButton.IsChecked = !this.startStopButton.IsChecked;
                    this.startStop();
                    e.Handled = true;
                    return;
                }
                case Key.Escape:
                {
                    if (this.isRunning || this.editModeProjectLabel.ViewModel?.HasProject != true)
                        return;
                    this.clearSelectedProject();
                    e.Handled = true;
                    return;
                }
            }
        }

        private void onProjectLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.tryOpenEditViewIfRunning(e, Toggl.Project);
        }

        private void onDescriptionLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.tryOpenEditViewIfRunning(e, Toggl.Description);
        }

        private void onTimeLabelMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.tryOpenEditViewIfRunning(e, Toggl.Duration);
        }

        private void onGridMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.tryOpenEditViewIfRunning(e, "");
        }

        private void descriptionAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asItem = e as TimerItem;
            if (asItem == null)
                return;

            var item = asItem.Item;

            this.descriptionTextBox.SetText(item.Description);

            this.editProjectPanel.ShowOnlyIf(item.ProjectID != 0);
            this.editModeProjectLabel.ViewModel = item.ToProjectLabelViewModel();
            completedProject = item;
        }

        private void cancelProjectSelectionButtonClick(object sender, RoutedEventArgs e)
        {
            this.clearSelectedProject();
        }

        private void clearSelectedProject()
        {
            this.editProjectPanel.Visibility = Visibility.Collapsed;
            this.editModeProjectLabel.ViewModel = null;
            completedProject = default;
        }

        private void onManualAddButtonClick(object sender, RoutedEventArgs e)
        {
            var guid = Toggl.Start("", "0", 0, 0, "", "", IsMiniTimer);
            Toggl.Edit(guid, false, Toggl.Duration);
        }

        protected override void OnGotKeyboardFocus(KeyboardFocusChangedEventArgs e)
        {
            if (!this.IsKeyboardFocused)
                return;

            if (this.manualPanel.IsVisible)
            {
                this.manualAddButton.Focus();
            }
            else if (this.descriptionTextBox.IsVisible)
            {
                this.descriptionTextBox.Focus();
            }
            else
            {
                this.startStopButton.Focus();
            }
        }

        #endregion

        #region controlling

        private void startStop()
        {
            this.acceptNextUpdate = true;

            if (this.isRunning)
            {
                this.stop();
            }
            else
            {
                this.start();
            }
        }

        private void tryOpenEditViewIfRunning(MouseButtonEventArgs e, string focusedField)
        {
            if (this.isRunning)
            {
                if (e.ClickCount == 2)
                {
                    using (Performance.Measure("opening edit view from timer, focussing " + focusedField))
                    {
                        Toggl.Edit(this.runningTimeEntry.GUID, false, focusedField);
                    }

                    e.Handled = true;
                }
            }
        }

        private void start()
        {
            using (Performance.Measure("starting time entry from timer"))
            {
                var guid = Toggl.Start(
                    this.descriptionTextBox.Text,
                    "",
                    completedProject.TaskID,
                    completedProject.ProjectID,
                    "",
                    completedProject.Tags,
                    IsMiniTimer
                );

                if (completedProject.Billable)
                {
                    Toggl.SetTimeEntryBillable(guid, true);
                }
            }
        }

        private void stop()
        {
            using (Performance.Measure("stopping time entry from timer"))
            {
                Toggl.Stop(IsMiniTimer);
            }
        }

        #endregion

        #region updating ui

        private void setUIToRunningState(Toggl.TogglTimeEntryView item)
        {
            this.resetUIState(true);
            this.timeEntryLabel.ViewModel = item.ToTimeEntryLabelViewModel();
            this.durationLabel.Text = Toggl.FormatDurationInSecondsHHMMSS(item.DurationInSeconds);
            this.durationPanel.ToolTip = "started at " + item.StartTimeString;
        }

        private void resetUIState(bool running, bool forceUpdate = false)
        {
            var changedState = this.isRunning != running;

            if (!(changedState || forceUpdate || this.acceptNextUpdate))
                return;

            this.acceptNextUpdate = false;

            this.isRunning = running;
            this.startStopButton.IsChecked = running;
            this.descriptionTextBox.SetText("");
            this.descriptionTextBox.ShowOnlyIf(!running);
            this.timeEntryLabel.ShowOnlyIf(running);
            this.durationPanel.ShowOnlyIf(running);
            this.editProjectPanel.Visibility = Visibility.Collapsed;
            this.editModeProjectLabel.ViewModel = null;
        }

        #endregion

        public void SetManualMode(bool manualMode)
        {
            this.manualPanel.ShowOnlyIf(manualMode);
            this.timerPanel.ShowOnlyIf(!manualMode);
        }

        private void MiniTimer_OnPreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (!startStopButton.IsMouseOver)
            {
                MouseCaptured?.Invoke(sender, e);
            }
        }
    }
}