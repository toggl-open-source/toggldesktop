
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion;
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
            Toggl.OnProjectAutocomplete += this.onProjectAutocomplete;

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
                this.projectTextBox.SetText(timeEntry.ProjectLabel);
                this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);
            }
            else
            {
                setTextIfUnfocused(this.descriptionTextBox, timeEntry.Description);
                setTextIfUnfocused(this.durationTextBox, timeEntry.Duration);
                setTimeIfUnfocused(this.startTimeTextBox, timeEntry.StartTimeString);
                setTimeIfUnfocused(this.endTimeTextBox, timeEntry.EndTimeString);
                setTextIfUnfocused(this.projectTextBox, timeEntry.ProjectLabel);

                if (!this.startDatePicker.IsFocused)
                    this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);
            }

            if (timeEntry.UpdatedAt > 0)
            {
                var updatedAt = Toggl.DateTimeFromUnix(timeEntry.UpdatedAt);
                this.lastUpdatedText.Text = "Last update " + updatedAt.ToShortDateString() + " at " + updatedAt.ToLongTimeString();
                this.lastUpdatedText.Visibility = Visibility.Visible;
            }
            else
            {
                this.lastUpdatedText.Visibility = Visibility.Collapsed;
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

        private void onProjectAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            this.projectAutoComplete.SetController(ProjectAutoCompleteController.From(list));
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

        #region description

        private void descriptionAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asDescriptionItem = e as DescriptionAutoCompleteItem;
            if (asDescriptionItem == null)
                return;

            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply description change: No time entry.");
                return;
            }

            var item = asDescriptionItem.Item;

            // TODO: fill in project already if possible (instead of waiting for dll)?

            Toggl.SetTimeEntryDescription(this.timeEntry.GUID, item.Description);
            Toggl.SetTimeEntryProject(this.timeEntry.GUID, item.TaskID, item.ProjectID, "");
        }

        private void descriptionAutoComplete_OnConfirmWithoutCompletion(object sender, string text)
        {
            this.setDescriptionIfChanged(text);
        }

        private void descriptionTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.setDescriptionIfChanged(this.descriptionTextBox.Text);
        }

        private void setDescriptionIfChanged(string text)
        {
            if (text == null || text == this.timeEntry.Description)
                return;

            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply description change: No time entry.");
                return;
            }

            Toggl.SetTimeEntryDescription(this.timeEntry.GUID, text);
        }

        #endregion

        #region project

        private void projectDropDownButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.projectAutoComplete.IsOpen = this.projectDropDownButton.IsChecked ?? false;

            if (!this.projectTextBox.IsKeyboardFocused)
            {
                this.projectTextBox.Focus();
                this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
                if (this.projectAutoComplete.IsOpen)
                {
                    this.projectTextBox.SelectAll();
                }
            }
        }

        private void projectAutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
            this.projectDropDownButton.IsChecked = this.projectAutoComplete.IsOpen;
        }

        private void projectAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asProjectItem = e as ProjectAutoCompleteItem;
            if (asProjectItem == null)
                return;

            var item = asProjectItem.Item;

            if (item.ProjectID == this.timeEntry.PID && item.TaskID == this.timeEntry.TID)
                return;

            this.projectTextBox.SetText(item.ProjectLabel);

            Toggl.SetTimeEntryProject(this.timeEntry.GUID, item.TaskID, item.ProjectID, "");
        }

        private void projectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            // TODO: reset project? add new? switch to 'add project mode'?
        }

        private void projectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.projectTextBox.SetText(this.timeEntry.ProjectLabel);
        }

        #endregion

        #endregion
    }
}
