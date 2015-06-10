
using System;
using System.Windows.Controls;
using System.Windows.Threading;

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
            InitializeComponent();

            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;

            this.durationUpdateTimer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(1),
                IsEnabled = true,
            };
            durationUpdateTimer.Tick += durationUpdateTimerTick;
        }

        #region helper methods

        private bool invoke(Action action)
        {
            if (this.Dispatcher.CheckAccess())
                return false;
            this.Dispatcher.Invoke(action);
            return true;
        }

        private static void setTextIfUnfocused(TextBox textBox, string text)
        {
            if (textBox.IsFocused)
                return;
            textBox.Text = text;
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

            if (open)
            {
                this.descriptionTextBox.Text = timeEntry.Description;
                this.durationTextBox.Text = timeEntry.Duration;
                this.startTimeTextBox.Text = timeEntry.StartTimeString;
                this.endTimeTextBox.Text = timeEntry.EndTimeString;
                this.projectComboBox.SelectedItem = timeEntry.ProjectLabel;
                this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);
            }
            else
            {
                setTextIfUnfocused(this.descriptionTextBox, timeEntry.Description);
                setTextIfUnfocused(this.durationTextBox, timeEntry.Duration);
                setTextIfUnfocused(this.startTimeTextBox, timeEntry.StartTimeString);
                setTextIfUnfocused(this.endTimeTextBox, timeEntry.EndTimeString);

                if (!this.projectComboBox.IsFocused)
                    this.projectComboBox.SelectedItem = timeEntry.ProjectLabel;

                if (!this.startDatePicker.IsFocused)
                    this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);
            }
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

        #endregion
    }
}
