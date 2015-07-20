using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;

namespace TogglDesktop.WPF
{
    public partial class TimerEditViewController
    {
        private readonly DispatcherTimer secondsTimer = new DispatcherTimer();
        private Toggl.TimeEntry runningTimeEntry;

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

            // TODO: populate auto complete
        }

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

        #endregion

        #region ui events

        private void startStopButtonOnClick(object sender, RoutedEventArgs e)
        {
            this.startStop();
        }

        private void onGridKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.startStopButton.IsChecked = !this.startStopButton.IsChecked;
                this.startStop();
                e.Handled = true;
            }
        }

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

        private void tryOpenEditViewIfRunning(MouseButtonEventArgs e, string focusedField)
        {
            if (this.isRunning)
            {
                Toggl.Edit(this.runningTimeEntry.GUID, false, focusedField);
                e.Handled = true;
            }
        }


        #endregion

        #region controlling

        private void start()
        {
            var description = this.descriptionTextBox.Text;
            var duration = this.durationTextBox.Text;

            var success = Toggl.Start(description, duration, 0, 0, "") != null;

            if (success)
            {
                //TODO: reset any stored data from auto completion if needed
            }
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
            this.projectLabel.Text = item.ClientLabel != "" ? "• " + item.ProjectLabel : item.ProjectLabel;
            setOptionalTextBlockText(this.clientLabel, item.ClientLabel);
            setOptionalTextBlockText(this.taskLabel, item.TaskLabel);

            this.projectLabel.Foreground = getProjectColorBrush(ref item);

            showOnlyIf(this.billabeIcon, item.Billable);
            showOnlyIf(this.tagsIcon, !string.IsNullOrEmpty(item.Tags));

            this.projectGridRow.Height = string.IsNullOrEmpty(item.ProjectLabel)
                ? new GridLength(0)
                : new GridLength(1, GridUnitType.Star);
        }

        private void setUIToStoppedState()
        {
            this.resetUIState(false);

            this.durationLabel.Text = "00:00:00";
        }

        private void resetUIState(bool running)
        {
            this.startStopButton.IsChecked = running;
            this.descriptionTextBox.Text = "";
            this.durationTextBox.Text = "";
            showOnlyIf(this.descriptionTextBox, !running);
            showOnlyIf(this.durationTextBox, !running);
            showOnlyIf(this.descriptionProjectGrid, running);
            showOnlyIf(this.iconPanel, running);
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
            var colourString = string.IsNullOrEmpty(item.Color) ? "#999999" : item.Color;
            var color = (Color)(ColorConverter.ConvertFromString(colourString) ?? Color.FromRgb(153, 153, 153));
            return new SolidColorBrush(color);
        }

        #endregion

        #endregion

        public bool CanFocusList()
        {
            return !this.descriptionTextBox.IsKeyboardFocused && !this.durationTextBox.IsKeyboardFocused;
        }
    }
}
