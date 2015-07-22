using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;

namespace TogglDesktop.WPF
{
    public partial class TimerEditViewController
    {
        private readonly DispatcherTimer secondsTimer = new DispatcherTimer();
        private Toggl.TimeEntry runningTimeEntry;
        private ProjectInfo completedProject;

        public event EventHandler RunningTimeEntrySecondPulse;

        public TimerEditViewController()
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

        #region helper methods

        private bool invoke(Action action)
        {
            if (this.Dispatcher.CheckAccess())
                return false;
            this.Dispatcher.Invoke(action);
            return true;
        }

        #endregion

        #region toggl events

        private void onStoppedTimerState()
        {
            if (this.invoke(this.onStoppedTimerState))
                return;

            this.secondsTimer.IsEnabled = false;
            this.setUIToStoppedState();
            this.runningTimeEntry = default(Toggl.TimeEntry);
        }

        private void onRunningTimerState(Toggl.TimeEntry te)
        {
            if (this.invoke(() => this.onRunningTimerState(te)))
                return;

            this.runningTimeEntry = te;
            this.setUIToRunningState(te);
            this.secondsTimer.IsEnabled = true;
        }

        private void onMiniTimerAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            if (this.invoke(() => this.onMiniTimerAutocomplete(list)))
                return;

            this.descriptionAutoComplete.SetController(AutoCompleteControllers.ForTimer(list));
        }

        #endregion

        #region ui events

        private void timerTick(object sender, EventArgs e)
        {
            if (!this.isRunning)
                return;

            var s = Toggl.FormatDurationInSecondsHHMMSS(this.runningTimeEntry.DurationInSeconds);
            if (this.durationLabel.Text != s)
            {
                this.durationLabel.Text = s;
            }
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

                setOptionalTextBlockText(this.taskLabel, item.TaskLabel);
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
        }

        #endregion

        #region controlling

        private void startStop()
        {
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
                Toggl.Edit(this.runningTimeEntry.GUID, false, focusedField);
                e.Handled = true;
            }
        }

        private void start()
        {
            Toggl.Start(
                this.descriptionTextBox.Text,
                this.durationTextBox.Text,
                this.completedProject.TaskId,
                this.completedProject.ProjectId,
                ""
                );
        }

        private void stop()
        {
            Toggl.Stop();
        }

        #endregion 

        #region updating ui

        private void setUIToRunningState(Toggl.TimeEntry item)
        {
            this.resetUIState(true);

            this.descriptionLabel.Text = item.Description == "" ? "(no description)" : item.Description;
            this.projectLabel.Text = string.IsNullOrEmpty(item.ProjectLabel) ? "" : "• " + item.ProjectLabel;
            setOptionalTextBlockText(this.clientLabel, item.ClientLabel);
            setOptionalTextBlockText(this.taskLabel, item.TaskLabel);

            this.projectLabel.Foreground = getProjectColorBrush(ref item);

            showOnlyIf(this.billabeIcon, item.Billable);
            showOnlyIf(this.tagsIcon, !string.IsNullOrEmpty(item.Tags));

            if (!string.IsNullOrEmpty(item.ProjectLabel))
                this.projectGridRow.Height = new GridLength(1, GridUnitType.Star);
        }

        private void setUIToStoppedState()
        {
            this.resetUIState(false);

            this.descriptionLabel.Text = "";
            this.durationLabel.Text = "00:00:00";
        }

        private void resetUIState(bool running)
        {
            this.startStopButton.IsChecked = running;
            this.descriptionTextBox.SetText("");
            this.durationTextBox.Text = "";
            showOnlyIf(this.descriptionTextBox, !running);
            showOnlyIf(this.durationTextBox, !running);
            showOnlyIf(this.iconPanel, running);
            this.projectGridRow.Height = new GridLength(0);
            this.completedProject = new ProjectInfo();
            this.cancelProjectSelectionButton.Visibility = Visibility.Collapsed;
        }

        #region display helpers


        private static void setOptionalTextBlockText(TextBlock textBlock, string text)
        {
            textBlock.Text = text;
            showOnlyIf(textBlock, !string.IsNullOrEmpty(text));
        }

        private static void showOnlyIf(UIElement element, bool condition)
        {
            element.Visibility = condition ? Visibility.Visible : Visibility.Collapsed;
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.TimeEntry item)
        {
            return getProjectColorBrush(item.Color);
        }

        private static SolidColorBrush getProjectColorBrush(ref Toggl.AutocompleteItem item)
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

        public bool CanFocusList()
        {
            return !this.descriptionTextBox.IsKeyboardFocused && !this.durationTextBox.IsKeyboardFocused;
        }

        private struct ProjectInfo
        {
            private readonly ulong projectId;
            private readonly ulong taskId;

            public ulong ProjectId { get { return this.projectId; } }
            public ulong TaskId { get { return this.taskId; } }

            public ProjectInfo(Toggl.AutocompleteItem item)
            {
                this.projectId = item.ProjectID;
                this.taskId = item.TaskID;
            }
        }

    }
}
