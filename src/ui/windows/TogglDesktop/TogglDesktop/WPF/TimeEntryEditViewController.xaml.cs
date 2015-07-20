
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;
using TextBox = System.Windows.Controls.TextBox;

namespace TogglDesktop.WPF
{
    /// <summary>
    /// Interaction logic for TimeEntryEditViewController.xaml
    /// </summary>
    public partial class TimeEntryEditViewController
    {
        private Toggl.TimeEntry timeEntry;
        private bool newProjectModeEnabled = true;
        private bool newClientModeEnabled = true;
        private List<Toggl.AutocompleteItem> projects;
        private List<Toggl.Model> clients;
        private List<Toggl.Model> workspaces;
        private ulong selectedWorkspaceId;
        private string selectedWorkspaceName;
        private string selectedClientGUID;
        private string selectedClientName;

        public TimeEntryEditViewController()
        {
            this.DataContext = this;
            InitializeComponent();

            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnTimeEntryAutocomplete += this.onTimeEntryAutocomplete;
            Toggl.OnProjectAutocomplete += this.onProjectAutocomplete;
            Toggl.OnClientSelect += this.onClientSelect;
            Toggl.OnTags += this.onTags;
            Toggl.OnWorkspaceSelect += this.onWorkspaceSelect;
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

        #region from time entry

        private void onTimeEntryEditor(bool open, Toggl.TimeEntry timeEntry, string focusedFieldName)
        {
            if (this.invoke(() => this.onTimeEntryEditor(open, timeEntry, focusedFieldName)))
                return;

            this.timeEntry = timeEntry;

            var isCurrentlyRunning = timeEntry.DurationInSeconds < 0;

            this.endTimeTextBox.IsEnabled = !isCurrentlyRunning;

            if (open)
            {
                this.descriptionTextBox.SetText(timeEntry.Description);
                this.durationTextBox.Text = timeEntry.Duration;
                setTime(this.startTimeTextBox, timeEntry.StartTimeString);
                setTime(this.endTimeTextBox, timeEntry.EndTimeString);
                this.projectTextBox.SetText(timeEntry.ProjectLabel);
                this.clientTextBox.SetText(timeEntry.ClientLabel);
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

            if (isCurrentlyRunning)
            {
                this.endTimeTextBox.Text = "";
            }

            this.billableCheckBox.Visibility = timeEntry.CanSeeBillable ? Visibility.Visible : Visibility.Collapsed;
            this.billableCheckBox.IsChecked = timeEntry.Billable;

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

            this.tagList.Clear(open);
            if(timeEntry.Tags != null)
                this.tagList.AddTags(timeEntry.Tags.Split(new[] { Toggl.TagSeparator }, StringSplitOptions.RemoveEmptyEntries));
            this.updateTagListEmptyText();

            if (this.newProjectModeEnabled)
                this.disableNewProjectMode();

            this.projectColorCircle.Background = new SolidColorBrush(getProjectColor(timeEntry.Color));

            this.selectedWorkspaceId = timeEntry.WID;
            this.selectedWorkspaceName = timeEntry.WorkspaceName;

            if (timeEntry.CanAddProjects)
            {
                this.newProjectButton.Visibility = Visibility.Visible;
                this.projectAddButtonColumn.Width = GridLength.Auto;
                this.projectAddButtonColumn.SharedSizeGroup = "AddButtons";
            }
            else
            {
                this.newProjectButton.Visibility = Visibility.Hidden;
                this.projectAddButtonColumn.Width = new GridLength(0);
                this.projectAddButtonColumn.SharedSizeGroup = null;
            }

        }

        private static Color getProjectColor(string colorString)
        {
            var projectColourString = string.IsNullOrEmpty(colorString) ? "#999999" : colorString;
            var projectColor = (Color)(ColorConverter.ConvertFromString(projectColourString) ?? Color.FromRgb(153, 153, 153));
            return projectColor;
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

        #endregion

        #region duration auto update

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

        private void onTimeEntryAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            this.descriptionAutoComplete.SetController(AutoCompleteControllers.ForDescriptions(list));
        }

        private void onProjectAutocomplete(List<Toggl.AutocompleteItem> list)
        {
            this.projects = list;

            this.tryUpdatingProjectAutoComplete();
        }

        private void onClientSelect(List<Toggl.Model> list)
        {
            this.clients = list;

            this.tryUpdatingClientAutoComplete();
            this.tryUpdatingProjectAutoComplete();
        }

        private void tryUpdatingProjectAutoComplete()
        {
            if (this.projects == null || this.clients == null || this.workspaces == null)
                return;

            this.projectAutoComplete.SetController(
                AutoCompleteControllers.ForProjects(this.projects, this.clients, this.workspaces)
                );
        }
        private void tryUpdatingClientAutoComplete()
        {
            if (this.clients == null || this.workspaces == null)
                return;

            this.clientAutoComplete.SetController(
                AutoCompleteControllers.ForClients(this.clients, this.workspaces)
                );
        }

        private void onTags(List<Toggl.Model> list)
        {
            this.tagList.SetKnownTags(list.Select(m => m.Name));
        }

        private void onWorkspaceSelect(List<Toggl.Model> list)
        {
            this.workspaces = list;
            this.workspaceAutoComplete.SetController(AutoCompleteControllers.ForWorkspaces(list));

            this.tryUpdatingClientAutoComplete();
            this.tryUpdatingProjectAutoComplete();
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
            this.applyDateChangeOrReset();
        }

        private void startDatePicker_OnLostFocus(object sender, RoutedEventArgs e)
        {
            this.applyDateChangeOrReset();
        }

        private void applyDateChangeOrReset()
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
            var asDescriptionItem = e as DescriptionItem;
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
            //TODO: fix clicking this to close reopens due to popup-capture->close-event->button-click
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
            var asProjectItem = e as ProjectItem;
            if (asProjectItem == null)
                return;

            var item = asProjectItem.Item;

            this.setProjectIfDifferent(item.TaskID, item.ProjectID, item.ProjectLabel);
        }

        private void projectAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            if (this.projectTextBox.Text == "")
            {
                this.setProjectIfDifferent(0, 0, "");
            }
            else
            {
                // TODO: reset project? add new? switch to 'add project mode'?   
            }
        }

        private void projectTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if(this.newProjectModeEnabled)
                return;
            
            if (this.projectTextBox.Text == "")
            {
                this.setProjectIfDifferent(0, 0, "");
            }
            else
            {
                // TODO: if only one entry is left in auto complete box, should it be selected?

                this.projectTextBox.SetText(this.timeEntry.ProjectLabel);
            }

        }

        private void setProjectIfDifferent(ulong taskId, ulong projectId, string projectName)
        {
            if (projectId == this.timeEntry.PID && taskId == this.timeEntry.TID)
                return;
            this.projectTextBox.Text = projectName;
            Toggl.SetTimeEntryProject(this.timeEntry.GUID, taskId, projectId, "");
        }

        private void newProjectButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.enableNewProjectMode();
        }
        private void newProjectCancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.disableNewProjectMode();
        }

        #endregion

        #region new project mode

        private void enableNewProjectMode()
        {
            this.showClientArea();

            this.projectTextBox.SetValue(Grid.ColumnSpanProperty, 2);
            this.projectAutoComplete.IsEnabled = false;
            this.projectDropDownButton.Visibility = Visibility.Hidden;
            this.newProjectButton.Visibility = Visibility.Hidden;
            this.newProjectCancelButton.Visibility = Visibility.Visible;
            this.projectTextBox.Focus();
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;

            this.projectColorCircle.Visibility = Visibility.Collapsed;
            this.projectTextBox.Padding = new Thickness(8, 0, 34, 0);
            this.emptyProjectText.Text = "Add project";
            this.emptyProjectText.Margin = new Thickness(16, 0, 16, 0);

            this.newProjectModeEnabled = true;
        }

        private void disableNewProjectMode()
        {
            this.disableNewClientMode();
            this.hideClientArea();

            this.projectTextBox.SetValue(Grid.ColumnSpanProperty, 1);
            this.projectAutoComplete.IsEnabled = true;
            this.projectDropDownButton.Visibility = Visibility.Visible;
            this.newProjectButton.Visibility = Visibility.Visible;
            this.newProjectCancelButton.Visibility = Visibility.Hidden;
            this.projectTextBox.Focus();
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;

            this.projectColorCircle.Visibility = Visibility.Visible;
            this.projectTextBox.Padding = new Thickness(28, 0, 34, 0);
            this.emptyProjectText.Text = "No project";
            this.emptyProjectText.Margin = new Thickness(36, 0, 16, 0);

            this.newProjectModeEnabled = false;
        }

        private void projectTextBox_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.newProjectModeEnabled)
                return;

            switch (e.Key)
            {
                case Key.Escape:
                    {
                        this.disableNewProjectMode();
                        break;
                    }
                case Key.Enter:
                    {
                        if (this.tryCreatingNewProject(this.projectTextBox.Text))
                            this.disableNewProjectMode();
                        break;
                    }
            }
        }

        private bool tryCreatingNewProject(string text)
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot add new project: No time entry.");
                return false;
            }

            var projectGUID = Toggl.AddProject(this.timeEntry.GUID, this.selectedWorkspaceId, 0, this.selectedClientGUID, text, false);

            return !string.IsNullOrEmpty(projectGUID);
        }

        #endregion

        #region client

        private void showClientArea()
        {
            this.clientTextBox.Text = "";
            this.selectedClientName = "";
            this.selectedClientGUID = "";
            this.clientAutoComplete.IsOpen = false;
            this.clientArea.Visibility = Visibility.Visible;
        }

        private void hideClientArea()
        {
            this.clientArea.Visibility = Visibility.Collapsed;
            this.clientAutoComplete.IsOpen = false;
        }

        private void clientDropDownButton_OnClick(object sender, RoutedEventArgs e)
        {
            //TODO: fix clicking this to close reopens due to popup-capture->close-event->button-click
            this.clientAutoComplete.IsOpen = this.clientDropDownButton.IsChecked ?? false;

            if (!this.clientTextBox.IsKeyboardFocused)
            {
                this.clientTextBox.Focus();
                this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;
                if (this.clientAutoComplete.IsOpen)
                {
                    this.clientTextBox.SelectAll();
                }
            }
        }

        private void clientAutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
            this.clientDropDownButton.IsChecked = this.clientAutoComplete.IsOpen;
        }

        private void clientAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asClientItem = e as ModelItem;
            if (asClientItem == null)
                return;

            var item = asClientItem.Item;

            this.selectClient(item);

            this.projectTextBox.Focus();
        }

        private void selectClient(Toggl.Model item)
        {
            this.selectedClientGUID = item.GUID;
            this.selectedClientName = item.Name;
            this.clientTextBox.SetText(item.Name);
        }

        private void clientAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            if (this.clientTextBox.Text == "")
            {
                this.selectClient(new Toggl.Model());
            }
            else
            {
                // TODO: reset client? add new? switch to 'add new client mode'?
            }
        }

        private void clientTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this.newClientModeEnabled)
                return;

            if (this.clientTextBox.Text == "")
            {
                this.selectClient(new Toggl.Model());
            }
            else
            {
                // TODO: if only one entry is left in auto complete box, should it be selected?

                this.clientTextBox.SetText(this.selectedClientName);
            }

        }

        private void newClientButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.enableNewClientMode();
        }

        private void newClientCancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.disableNewClientMode();

            this.projectTextBox.Focus();
        }
        #endregion

        #region new client mode


        private void enableNewClientMode()
        {
            this.clientTextBox.SetValue(Grid.ColumnSpanProperty, 2);
            this.clientAutoComplete.IsEnabled = false;
            this.clientDropDownButton.Visibility = Visibility.Hidden;
            this.newClientButton.Visibility = Visibility.Hidden;
            this.newClientCancelButton.Visibility = Visibility.Visible;
            this.clientTextBox.Focus();
            this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;
            this.showWorkspaceArea();

            this.emptyClientText.Text = "Add client";

            this.newClientModeEnabled = true;
        }

        private void disableNewClientMode()
        {
            this.clientTextBox.SetValue(Grid.ColumnSpanProperty, 1);
            this.clientAutoComplete.IsEnabled = true;
            this.clientDropDownButton.Visibility = Visibility.Visible;
            this.newClientButton.Visibility = Visibility.Visible;
            this.newClientCancelButton.Visibility = Visibility.Hidden;
            this.clientTextBox.Focus();
            this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;
            this.hideWorkspaceArea();

            this.emptyClientText.Text = "No client";

            this.newClientModeEnabled = false;
        }

        private void clientTextBox_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.newClientModeEnabled)
                return;

            switch (e.Key)
            {
                case Key.Escape:
                    {
                        this.disableNewClientMode();
                        break;
                    }
                case Key.Enter:
                    {
                        if (this.tryCreatingNewClient(this.clientTextBox.Text))
                        {
                            this.disableNewClientMode();

                            this.projectTextBox.Focus();
                        }
                        break;
                    }
            }
        }

        private bool tryCreatingNewClient(string text)
        {
            var clientGUID = Toggl.CreateClient(this.selectedWorkspaceId, text);

            Console.WriteLine("client guid: " + (clientGUID ?? "null"));

            if (string.IsNullOrEmpty(clientGUID))
                return false;

            this.selectedClientName = text;
            this.selectedClientGUID = clientGUID;

            this.disableNewClientMode();

            this.projectTextBox.Focus();

            return true;
        }

        #endregion

        #region workspace

        private void showWorkspaceArea()
        {
            this.workspaceTextBox.Text = this.selectedWorkspaceName;
            this.workspaceAutoComplete.IsOpen = false;
            this.workspaceArea.Visibility = Visibility.Visible;
        }

        private void hideWorkspaceArea()
        {
            this.workspaceArea.Visibility = Visibility.Collapsed;
            this.workspaceAutoComplete.IsOpen = false;
        }

        private void workspaceDropDownButton_OnClick(object sender, RoutedEventArgs e)
        {
            //TODO: fix clicking this to close reopens due to popup-capture->close-event->button-click
            this.workspaceAutoComplete.IsOpen = this.workspaceDropDownButton.IsChecked ?? false;

            if (!this.workspaceTextBox.IsKeyboardFocused)
            {
                this.workspaceTextBox.Focus();
                this.workspaceTextBox.CaretIndex = this.workspaceTextBox.Text.Length;
                if (this.workspaceAutoComplete.IsOpen)
                {
                    this.workspaceTextBox.SelectAll();
                }
            }
        }

        private void workspaceAutoComplete_OnIsOpenChanged(object sender, EventArgs e)
        {
            this.workspaceDropDownButton.IsChecked = this.workspaceAutoComplete.IsOpen;
        }

        private void workspaceAutoComplete_OnConfirmCompletion(object sender, AutoCompleteItem e)
        {
            var asWorkspaceItem = e as ModelItem;
            if (asWorkspaceItem == null)
                return;

            var item = asWorkspaceItem.Item;

            this.selectWorkspace(item);

            this.clientTextBox.Focus();
        }

        private void selectWorkspace(Toggl.Model item)
        {
            this.selectedWorkspaceId = item.ID;
            this.selectedWorkspaceName = item.Name;
            this.workspaceTextBox.SetText(item.Name);
        }

        private void workspaceAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            // TODO: reset client? add new? switch to 'add new client mode'?
        }

        private void workspaceTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.workspaceTextBox.SetText(this.selectedWorkspaceName);
        }
        #endregion


        #region tag list

        private void tagList_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot change tags: No time entry.");
                return;
            }

            this.updateTagListEmptyText();

            Toggl.SetTimeEntryTags(this.timeEntry.GUID, this.tagList.Tags.ToList());
        }

        private void tagList_OnGotKeybardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.updateTagListEmptyText();
        }

        private void updateTagListEmptyText()
        {
            this.emptyTagListText.Visibility = this.tagList.TagCount == 0 && !this.tagList.IsKeyboardFocusWithin
                ? Visibility.Visible : Visibility.Collapsed;
        }

        #endregion

        private void billableCheckBox_OnClick(object sender, RoutedEventArgs e)
        {
            Toggl.SetTimeEntryBillable(this.timeEntry.GUID, this.billableCheckBox.IsChecked ?? false);
        }

        #endregion

        #region variuos

        public void SetTimer(TimerEditViewController timer)
        {
            timer.RunningTimeEntrySecondPulse += this.durationUpdateTimerTick;
        }

        public void FocusField(string focusedFieldName)
        {
            switch (focusedFieldName)
            {
                case Toggl.Project:
                    this.projectTextBox.Focus();
                    break;
                case Toggl.Duration:
                    this.durationTextBox.Focus();
                    break;
                case Toggl.Description:
                    this.descriptionTextBox.Focus();
                    break;
            }
        }

        private void backButton_OnClick(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        public void Close()
        {
            //TODO: make sure unsaved changes are discarded/saved (what if user is in add-project mode?)
            Toggl.ViewTimeEntryList();
            //TODO: reset form (specifically add-project controls)?
        }

        private void deleteButton_OnClick(object sender, RoutedEventArgs e)
        {
            DialogResult result;
            try
            {
                MainWindowController.DisableTop();
                result = System.Windows.Forms.MessageBox.Show("Delete time entry?", "Please confirm",
                                     MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            }
            finally
            {
                MainWindowController.EnableTop();
            }
            if (DialogResult.Yes == result)
            {
                Toggl.DeleteTimeEntry(timeEntry.GUID);
                //TODO: reset form (specifically add-project controls)?
            }
        }

        public void SetShadow(bool left, int height)
        {
            this.shadowLeft.Visibility = left ? Visibility.Collapsed : Visibility.Visible;
            this.shadowRight.Visibility = left ? Visibility.Visible : Visibility.Collapsed;

            this.shadowLeft.MinHeight = height;
            this.shadowRight.MinHeight = height;
        }
        #endregion

    }
}
