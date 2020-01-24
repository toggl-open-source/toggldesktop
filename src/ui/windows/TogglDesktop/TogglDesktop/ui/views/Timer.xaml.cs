using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;

namespace TogglDesktop
{
    public partial class Timer
    {
        private readonly DispatcherTimer secondsTimer = new DispatcherTimer();
        private Toggl.TogglTimeEntryView runningTimeEntry;
        private bool isRunning;
        private bool acceptNextUpdate;

        public event EventHandler StartStopClick;
        public event EventHandler<string> RunningTimeEntrySecondPulse;
        public event EventHandler FocusTimeEntryList;
        public event EventHandler<string> DescriptionTextBoxTextChanged;

        public Timer()
        {
            this.InitializeComponent();

            this.setupSecondsTimer();

            Toggl.OnMinitimerAutocomplete += this.onMiniTimerAutocomplete;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;

            this.RunningTimeEntrySecondPulse += this.timerTick;

            this.resetUIState(false, true);
        }

        private static bool IsMiniTimer => false;

        private void setupSecondsTimer()
        {
            this.secondsTimer.Interval = TimeSpan.FromSeconds(1);
            this.secondsTimer.Tick += (sender, args) =>
            {
                if (!this.isRunning)
                    return;

                var s = Toggl.FormatDurationInSecondsHHMMSS(this.runningTimeEntry.DurationInSeconds);

                if (this.RunningTimeEntrySecondPulse != null)
                    this.RunningTimeEntrySecondPulse(this, s);
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

        private void timerTick(object sender, string t)
        {
            this.runningEntryInfoPanel.SetDurationLabel(t);
        }

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
                    if (this.isRunning || this.editModeProjectLabel.ViewModel.HasProject == false)
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
            this.editModeProjectLabel.ViewModel.SetProject(item);

            this.runningEntryInfoPanel.OnConfirmCompletion(item);
        }

        private void cancelProjectSelectionButtonClick(object sender, RoutedEventArgs e)
        {
            this.clearSelectedProject();
        }

        private void clearSelectedProject()
        {
            this.timeEntryLabel.ClearProject();
            this.editProjectPanel.Visibility = Visibility.Collapsed;
            this.editModeProjectLabel.ViewModel.Clear();
        }

        private void onManualAddButtonClick(object sender, RoutedEventArgs e)
        {
            var guid = Toggl.Start("", "0", 0, 0, "", "", IsMiniTimer);
            Toggl.Edit(guid, false, Toggl.Duration);
        }

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            if (e.Key == Key.Down && Keyboard.Modifiers == System.Windows.Input.ModifierKeys.Shift)
            {
                if (this.FocusTimeEntryList != null)
                    this.FocusTimeEntryList(this, e);
                e.Handled = true;
            }

            base.OnPreviewKeyDown(e);
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
            if (this.StartStopClick != null)
                this.StartStopClick(this, EventArgs.Empty);

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
                using (Performance.Measure("opening edit view from timer, focussing " + focusedField))
                {
                    Toggl.Edit(this.runningTimeEntry.GUID, false, focusedField);
                }
                e.Handled = true;
            }
        }

        private void start()
        {
            using (Performance.Measure("starting time entry from timer"))
            {
                var completedProject = editModeProjectLabel.ViewModel.ProjectInfo;
                var guid = Toggl.Start(
                    this.descriptionTextBox.Text,
                    "",
                    completedProject.TaskId,
                    completedProject.ProjectId,
                    "",
                    this.runningEntryInfoPanel.TagsString,
                    IsMiniTimer
                    );

                if (this.runningEntryInfoPanel.IsBillable)
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
            this.timeEntryLabel.SetTimeEntry(item);
            this.runningEntryInfoPanel.SetUIToRunningState(item);
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
            this.runningEntryInfoPanel.ResetUIState(running);
            this.editModeProjectLabel.ViewModel.Clear();
            this.editProjectPanel.Visibility = Visibility.Collapsed;
        }

        #endregion

        public void SetManualMode(bool manualMode)
        {
            this.manualPanel.ShowOnlyIf(manualMode);
            this.timerPanel.ShowOnlyIf(!manualMode);
        }

        private void onDescriptionTextBoxTextChanged(object sender, TextChangedEventArgs e)
        {
            DescriptionTextBoxTextChanged?.Invoke(sender, this.descriptionTextBox.Text);
        }
    }
}