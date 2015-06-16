
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion.Implementation;

namespace TogglDesktop.WPF
{
    /// <summary>
    /// Interaction logic for TimeEntryEditViewController.xaml
    /// </summary>
    public partial class TimeEntryEditViewController
    {
        private readonly DispatcherTimer durationUpdateTimer;
        private Toggl.TimeEntry timeEntry;

        public TimeEntryEditViewController()
        {
            this.DataContext = this;
            InitializeComponent();

            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnTimeEntryAutocomplete += this.onTimeEntryAutocomplete;

            this.durationUpdateTimer = this.startDurationUpdateTimer();
        }

        #region helper methods

        private bool invoke(Action action)
        {
            if (this.Dispatcher.CheckAccess())
                return false;
            this.Dispatcher.Invoke(action);
            return true;
        }

        private bool hasTimeEntry()
        {
            return this.timeEntry.GUID != null;
        }

        #endregion

        #region fill with data

        private void onTimeEntryEditor(bool open, Toggl.TimeEntry timeEntry, string focusedFieldName)
        {
            if (this.invoke(() => this.onTimeEntryEditor(open, timeEntry, focusedFieldName)))
                return;

            this.timeEntry = timeEntry;
            this.durationUpdateTimer.IsEnabled = this.timeEntry.DurationInSeconds < 0;

            if (open)
            {
                this.descriptionTextBox.SetText(timeEntry.Description);
                this.durationTextBox.Text = timeEntry.Duration;
                setTime(this.startTimeTextBox, timeEntry.StartTimeString);
                setTime(this.endTimeTextBox, timeEntry.EndTimeString);
                this.projectComboBox.SelectedItem = timeEntry.ProjectLabel;
                this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);
            }
            else
            {
                setTextIfUnfocused(this.descriptionTextBox, timeEntry.Description);
                setTextIfUnfocused(this.durationTextBox, timeEntry.Duration);
                setTimeIfUnfocused(this.startTimeTextBox, timeEntry.StartTimeString);
                setTimeIfUnfocused(this.endTimeTextBox, timeEntry.EndTimeString);

                if (!this.projectComboBox.IsFocused)
                    this.projectComboBox.SelectedItem = timeEntry.ProjectLabel;

                if (!this.startDatePicker.IsFocused)
                    this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);
            }
        }

        private static void setTime(TextBox textBox, string time)
        {
            textBox.Text = time;
            textBox.Tag = time;
        }

        private static void setTimeIfUnfocused(TextBox textBox, string time)
        {
            if (textBox.IsFocused)
                return;
            textBox.Text = time;
            textBox.Tag = time;
        }
        private static void setTextIfUnfocused(TextBox textBox, string text)
        {
            if (textBox.IsFocused)
                return;
            textBox.Text = text;
        }
        private static void setTextIfUnfocused(ExtendedTextBox textBox, string text)
        {
            if (textBox.IsFocused)
                return;
            textBox.SetText(text);
        }

        private DispatcherTimer startDurationUpdateTimer()
        {
            var timer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(1),
            };
            timer.Tick += this.durationUpdateTimerTick;
            return timer;
        }

        private void durationUpdateTimerTick(object sender, EventArgs eventArgs)
        {
            if (!this.hasTimeEntry() || this.timeEntry.DurationInSeconds >= 0)
                return;

            if (this.durationTextBox.IsFocused)
                return;

            var s = Toggl.FormatDurationInSecondsHHMMSS(this.timeEntry.DurationInSeconds);
            if (this.durationTextBox.Text != s)
            {
                this.durationTextBox.Text = s;
            }
        }

        private void onTimeEntryAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            this.descriptionAutoComplete.SetController(DescriptionAutoCompleteController.From(list));
        }

        #endregion

        #region change data

        #region time and date

        private void startTimeTextBox_OnLostKeyboardFocus(object sender, RoutedEventArgs e)
        {
            this.setTimeEntryTimeIfChanged(this.startTimeTextBox, Toggl.SetTimeEntryStart, "start time");
        }

        private void endTimeTextBox_OnLostKeyboardFocus(object sender, RoutedEventArgs e)
        {
            this.setTimeEntryTimeIfChanged(this.endTimeTextBox, Toggl.SetTimeEntryEnd, "end time");
        }

        private void durationTextBox_OnLostKeyboardFocus(object sender, RoutedEventArgs e)
        {
            this.setTimeEntryTimeIfChanged(this.durationTextBox, Toggl.SetTimeEntryDuration, "duration");
        }

        private void setTimeEntryTimeIfChanged(TextBox textBox, Func<string, string, bool> apiCall, string timeType)
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply " + timeType + " change: No time entry.");
                return;
            }

            var before = textBox.Tag as string;
            var now = textBox.Text;
            if (before == now)
                return;

            apiCall(this.timeEntry.GUID, now);
        }

        private void startDatePicker_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply date change: No time entry.");
                return;
            }
            if (!this.startDatePicker.SelectedDate.HasValue)
            {
                this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(this.timeEntry.Started);
                return;
            }
            Toggl.SetTimeEntryDate(this.timeEntry.GUID, this.startDatePicker.SelectedDate.Value);
        }

        #endregion

        #endregion

    }
}
