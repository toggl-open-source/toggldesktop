using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;

namespace TogglDesktop.WPF
{
    public partial class Timer
    {
        private readonly DispatcherTimer secondsTimer = new DispatcherTimer();
        private Toggl.TogglTimeEntryView runningTimeEntry;
        private ProjectInfo completedProject;

        public event EventHandler StartStopClick;
        public event EventHandler RunningTimeEntrySecondPulse;
        public event EventHandler FocusTimeEntryList;

        public Timer()
        {
            this.InitializeComponent();

            this.setupSecondsTimer();

            Toggl.OnMinitimerAutocomplete += this.onMiniTimerAutocomplete;
            Toggl.OnRunningTimerState += this.onRunningTimerState;
            Toggl.OnStoppedTimerState += this.onStoppedTimerState;

            this.RunningTimeEntrySecondPulse += this.timerTick;

            this.setUIToStoppedState();
        }

        private void setupSecondsTimer()
        {
            this.secondsTimer.Interval = TimeSpan.FromSeconds(1);
            this.secondsTimer.Tick += (sender, args) =>
            {
                if (this.RunningTimeEntrySecondPulse != null)
                    this.RunningTimeEntrySecondPulse(this, EventArgs.Empty);
            };
        }

        private bool isRunning { get { return this.startStopButton.IsChecked ?? false; } }

        #region toggl events

        private void onStoppedTimerState()
        {
            if (this.TryBeginInvoke(this.onStoppedTimerState))
                return;

            using (Performance.Measure("timer responding to OnStoppedTimerState"))
            {
                this.secondsTimer.IsEnabled = false;
                this.setUIToStoppedState();
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

        private void timerTick(object sender, EventArgs e)
        {
            if (!this.isRunning)
                return;

            this.setRunningDurationLabels();
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
                    if (!this.isRunning)
                    {
                        this.setUIToStoppedState();
                        e.Handled = true;
                    }
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

            this.completedProject = new ProjectInfo(item);

            if (item.ProjectID != 0)
            {
                this.projectLabel.Text = "• " + item.ProjectLabel;
                this.projectLabel.Foreground = getProjectColorBrush(ref item);

                setOptionalTextBlockText(this.taskLabel, string.IsNullOrEmpty(item.TaskLabel) ? "" : item.TaskLabel + " -");
                setOptionalTextBlockText(this.clientLabel, item.ClientLabel);

                this.projectGridRow.Height = GridLength.Auto;
                this.cancelProjectSelectionButton.Visibility = Visibility.Visible;
            }
            else
            {
                this.projectGridRow.Height = new GridLength(0);
                this.cancelProjectSelectionButton.Visibility = Visibility.Collapsed;
            }
        }

        private void cancelProjectSelectionButtonClick(object sender, RoutedEventArgs e)
        {
            this.projectGridRow.Height = new GridLength(0);
            this.completedProject = new ProjectInfo();
            this.taskLabel.Visibility = Visibility.Collapsed;
        }

        private void onManualAddButtonClick(object sender, RoutedEventArgs e)
        {
            var guid = Toggl.Start("", "0", 0, 0, "", "");
            Toggl.Edit(guid, false, Toggl.Duration);
        }

        private void onFocusTimeEntryListCommand(object sender, ExecutedRoutedEventArgs e)
        {
            if (this.FocusTimeEntryList != null)
                this.FocusTimeEntryList(this, e);
        }

        #endregion

        #region controlling

        private void startStop()
        {
            if (this.StartStopClick != null)
                this.StartStopClick(this, EventArgs.Empty);

            if (this.isRunning)
            {
                this.start();
            }
            else
            {
                this.stop();
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
                Toggl.Start(
                    this.descriptionTextBox.Text,
                    this.durationTextBox.Text,
                    this.completedProject.TaskId,
                    this.completedProject.ProjectId,
                    "",
                    ""
                    );
            }
        }

        private void stop()
        {
            using (Performance.Measure("stopping time entry from timer"))
            {
                Toggl.Stop();
            }
        }

        #endregion 

        #region updating ui

        private void setUIToRunningState(Toggl.TogglTimeEntryView item)
        {
            this.resetUIState(true);

            this.descriptionLabel.Text = item.Description == "" ? "(no description)" : item.Description;
            this.projectLabel.Text = string.IsNullOrEmpty(item.ClientLabel) ? item.ProjectLabel : "• " + item.ProjectLabel;
            setOptionalTextBlockText(this.clientLabel, item.ClientLabel);
            setOptionalTextBlockText(this.taskLabel, string.IsNullOrEmpty(item.TaskLabel) ? "" : item.TaskLabel + " -");

            this.projectLabel.Foreground = getProjectColorBrush(ref item);

            this.billabeIcon.ShowOnlyIf(item.Billable);
            this.tagsIcon.ShowOnlyIf(!string.IsNullOrEmpty(item.Tags));

            if (!string.IsNullOrEmpty(item.ProjectLabel))
                this.projectGridRow.Height = new GridLength(1, GridUnitType.Star);

            this.setRunningDurationLabels();

            this.invalidate();
        }

        private void setRunningDurationLabels()
        {
            var seconds = this.runningTimeEntry.DurationInSeconds;
            var unixTimestamp = (long)(DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1))).TotalSeconds;
            var realSeconds = unixTimestamp + seconds;

            var s = Toggl.FormatDurationInSecondsHHMMSS(seconds);

            if (realSeconds < 10)
            {
                this.durationLabelLeft.Text = "";
                this.durationLabelRight.Text = s;
                return;
            }

            if (realSeconds >= 3600)
            {
                this.durationLabelLeft.Text = s;
                this.durationLabelRight.Text = "";
                this.durationLabelLeft.Margin = new Thickness(14, 0, 0, 0);
                return;
            }


            var split = s.Split(new[] {' '}, StringSplitOptions.RemoveEmptyEntries);

            if (split.Length != 2)
            {
                this.durationLabelLeft.Text = "";
                this.durationLabelRight.Text = s;

                Console.WriteLine("Warning: Running time entry time has unknown format.");
                return;
            }

            this.durationLabelLeft.Text = split[0];
            this.durationLabelRight.Text = split[1];

            this.durationLabelLeft.Margin = new Thickness(realSeconds < 60 ? 42 : 6, 0, 0, 0);

        }

        private void setUIToStoppedState()
        {
            this.resetUIState(false);

            this.descriptionLabel.Text = "What are you doing?";
            this.durationLabelLeft.Text = "00:00:00";
            this.durationLabelRight.Text = "";
            this.durationLabelLeft.Margin = new Thickness(14, 0, 0, 0);

            this.invalidate();
        }

        private void invalidate()
        {
            this.Dispatcher.Invoke(() => { }, DispatcherPriority.Render);
        }

        private void resetUIState(bool running)
        {
            this.startStopButton.IsChecked = running;
            this.descriptionTextBox.SetText("");
            this.durationTextBox.Text = "";
            this.descriptionTextBox.ShowOnlyIf(!running);
            this.durationTextBox.ShowOnlyIf(!running);
            this.iconPanel.ShowOnlyIf(running);
            this.projectGridRow.Height = new GridLength(0);
            this.completedProject = new ProjectInfo();
            this.cancelProjectSelectionButton.Visibility = Visibility.Collapsed;
            this.taskLabel.Visibility = Visibility.Collapsed;
        }

        #region display helpers


        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            textBlock.ShowOnlyIf(!string.IsNullOrEmpty(text));
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.TogglTimeEntryView item)
        {
            return getProjectColorBrush(item.Color);
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.TogglAutocompleteView item)
        {
            return getProjectColorBrush(item.ProjectColor);
        }

        private static SolidColorBrush getProjectColorBrush(string coplourString)
        {
            var colourString = string.IsNullOrEmpty(coplourString) ? "#999999" : coplourString;
            var color = (Color)(ColorConverter.ConvertFromString(colourString) ?? Color.FromRgb(153, 153, 153));
            return new SolidColorBrush(color);
        }

        #endregion

        #endregion

        private struct ProjectInfo
        {
            private readonly ulong projectId;
            private readonly ulong taskId;

            public ulong ProjectId { get { return this.projectId; } }
            public ulong TaskId { get { return this.taskId; } }

            public ProjectInfo(Toggl.TogglAutocompleteView item)
            {
                this.projectId = item.ProjectID;
                this.taskId = item.TaskID;
            }
        }

        public void SetManualMode(bool manualMode)
        {
            this.manualPanel.ShowOnlyIf(manualMode);
            this.timerPanel.ShowOnlyIf(!manualMode);
        }

    }
}
