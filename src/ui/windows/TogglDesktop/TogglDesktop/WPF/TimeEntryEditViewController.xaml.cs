
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using TogglDesktop.AutoCompletion;
using TogglDesktop.AutoCompletion.Implementation;
using TogglDesktop.Diagnostics;
using KeyEventArgs = System.Windows.Input.KeyEventArgs;
using TextBox = System.Windows.Controls.TextBox;

namespace TogglDesktop.WPF
{
    public partial class TimeEntryEditViewController
    {
        private Toggl.TogglTimeEntryView timeEntry;
        private bool isInNewProjectMode = true;
        private bool isInNewClientMode = true;
        private List<Toggl.TogglAutocompleteView> projects;
        private List<Toggl.TogglGenericView> clients;
        private List<Toggl.TogglGenericView> workspaces;
        private ulong selectedWorkspaceId;
        private string selectedWorkspaceName;
        private string selectedClientGUID;
        private string selectedClientName;

        public TimeEntryEditViewController()
        {
            this.DataContext = this;
            this.InitializeComponent();

            Toggl.OnLogin += this.onLogin;
            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
            Toggl.OnTimeEntryAutocomplete += this.onTimeEntryAutocomplete;
            Toggl.OnProjectAutocomplete += this.onProjectAutocomplete;
            Toggl.OnClientSelect += this.onClientSelect;
            Toggl.OnTags += this.onTags;
            Toggl.OnWorkspaceSelect += this.onWorkspaceSelect;
        }

        private void onLogin(bool open, ulong userId)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userId))
                return;

            this.timeEntry = new Toggl.TogglTimeEntryView();
            this.projects = null;
            this.clients = null;
            this.workspaces = null;
        }

        #region helper methods

        private bool hasTimeEntry()
        {
            return this.timeEntry.GUID != null;
        }

        #endregion

        #region fill with data

        #region from time entry

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView timeEntry, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, timeEntry, focusedFieldName))
                return;

            using (Performance.Measure("filling edit view from OnTimeEntryEditor"))
            {
                this.timeEntry = timeEntry;

                var isCurrentlyRunning = timeEntry.DurationInSeconds < 0;

                this.endTimeTextBox.IsEnabled = !isCurrentlyRunning;

                this.descriptionTextBox.SetText(timeEntry.Description);
                setTime(this.durationTextBox, timeEntry.Duration);
                setTime(this.startTimeTextBox, timeEntry.StartTimeString);
                setTime(this.endTimeTextBox, timeEntry.EndTimeString);
                this.projectTextBox.SetText(timeEntry.ProjectLabel);
                this.clientTextBox.SetText(timeEntry.ClientLabel);
                this.startDatePicker.SelectedDate = Toggl.DateTimeFromUnix(timeEntry.Started);

                if (isCurrentlyRunning)
                {
                    this.endTimeTextBox.Text = "";
                }

                this.billableCheckBox.Visibility = timeEntry.CanSeeBillable ? Visibility.Visible : Visibility.Collapsed;
                this.billableCheckBox.IsChecked = timeEntry.Billable;

                if (timeEntry.UpdatedAt > 0)
                {
                    var updatedAt = Toggl.DateTimeFromUnix(timeEntry.UpdatedAt);
                    this.lastUpdatedText.Text = "Last update " + updatedAt.ToShortDateString() + " at " +
                                                updatedAt.ToLongTimeString();
                    this.lastUpdatedText.Visibility = Visibility.Visible;
                }
                else
                {
                    this.lastUpdatedText.Visibility = Visibility.Collapsed;
                }

                if (open || !this.tagList.IsKeyboardFocusWithin)
                {
                    this.tagList.Clear(open);
                    if (timeEntry.Tags != null)
                        this.tagList.AddTags(timeEntry.Tags.Split(new[] {Toggl.TagSeparator},
                            StringSplitOptions.RemoveEmptyEntries));
                    this.updateTagListEmptyText();
                }

                if (this.isInNewProjectMode)
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
        }

        #region helpers

        private static Color getProjectColor(string colorString)
        {
            var projectColourString = string.IsNullOrEmpty(colorString) ? "#999999" : colorString;
            var projectColor = (Color)(ColorConverter.ConvertFromString(projectColourString) ?? Color.FromRgb(153, 153, 153));
            return projectColor;
        }

        private static void setTime(ExtendedTextBox textBox, string time)
        {
            textBox.SetText(time);
            textBox.Tag = time;
        }

        #endregion

        #endregion

        #region duration auto update

        private void durationUpdateTimerTick(object sender, EventArgs eventArgs)
        {
            if (!this.hasTimeEntry() || this.timeEntry.DurationInSeconds >= 0)
                return;

            if (this.durationTextBox.Text != (this.durationTextBox.Tag as string)
                || this.durationTextBox.SelectedText != "")
                return;

            var caret = this.durationTextBox.CaretIndex;

            var s = Toggl.FormatDurationInSecondsHHMMSS(this.timeEntry.DurationInSeconds);
            this.durationTextBox.Text = s;
            this.durationTextBox.Tag = s;

            this.durationTextBox.CaretIndex = caret;
        }

        #endregion

        #region auto completion

        private void onTimeEntryAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onTimeEntryAutocomplete, list))
                return;

            using (Performance.Measure("building edit view entry auto complete controller, {0} items", list.Count))
            {
                this.descriptionAutoComplete.SetController(AutoCompleteControllers.ForDescriptions(list));
            }
        }

        private void onProjectAutocomplete(List<Toggl.TogglAutocompleteView> list)
        {
            if (this.TryBeginInvoke(this.onProjectAutocomplete, list))
                return;

            this.projects = list;

            this.tryUpdatingProjectAutoComplete();
        }

        private void onClientSelect(List<Toggl.TogglGenericView> list)
        {
            if (this.TryBeginInvoke(this.onClientSelect, list))
                return;

            this.clients = list;

            this.tryUpdatingClientAutoComplete();
            this.tryUpdatingProjectAutoComplete();
        }

        private void tryUpdatingProjectAutoComplete()
        {
            if (this.projects == null || this.clients == null || this.workspaces == null)
                return;

            using (Performance.Measure("building edit view project auto complete controller, {0} items", this.projects.Count))
            {
                this.projectAutoComplete.SetController(
                    AutoCompleteControllers.ForProjects(this.projects, this.clients, this.workspaces)
                    );
            }
        }
        
        private void tryUpdatingClientAutoComplete()
        {
            if (this.clients == null || this.workspaces == null)
                return;

            using (Performance.Measure("building edit view client auto complete controller, {0} items", this.clients.Count))
            {
                this.clientAutoComplete.SetController(
                    AutoCompleteControllers.ForClients(this.clients, this.workspaces)
                    );
            }
        }

        private void onTags(List<Toggl.TogglGenericView> list)
        {
            if (this.TryBeginInvoke(this.onTags, list))
                return;

            using (Performance.Measure("building edit view tags auto complete controller, {0} items", list.Count))
            {
                this.tagList.SetKnownTags(list.Select(m => m.Name));
            }
        }

        private void onWorkspaceSelect(List<Toggl.TogglGenericView> list)
        {
            if (this.TryBeginInvoke(this.onWorkspaceSelect, list))
                return;

            this.workspaces = list;

            using (Performance.Measure("building edit view workspace auto complete controller, {0} items", list.Count))
            {
                this.workspaceAutoComplete.SetController(AutoCompleteControllers.ForWorkspaces(list));
            }

            this.tryUpdatingClientAutoComplete();
            this.tryUpdatingProjectAutoComplete();
        }

        #endregion

        #endregion

        #region change data

        #region time and date

        private void startTimeTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.saveStartTimeIfChanged();
        }

        private void endTimeTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.saveEndTimeIfChanged();
        }

        private void durationTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.saveDurationIfChanged();
        }

        private void startTimeTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.saveStartTimeIfChanged();
                e.Handled = true;
            }
        }

        private void endTimeTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.saveEndTimeIfChanged();
                e.Handled = true;
            }
        }

        private void durationTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.saveDurationIfChanged();
                e.Handled = true;
            }
        }

        private void saveStartTimeIfChanged()
        {
            this.setTimeEntryTimeIfChanged(this.startTimeTextBox, Toggl.SetTimeEntryStart, "start time");
        }
        private void saveEndTimeIfChanged()
        {
            this.setTimeEntryTimeIfChanged(this.endTimeTextBox, Toggl.SetTimeEntryEnd, "end time");
        }
        private void saveDurationIfChanged()
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
            var asTimerItem = e as TimerItem;
            if (asTimerItem == null)
                return;

            if (!this.hasTimeEntry())
            {
                Console.WriteLine("Cannot apply description change: No time entry.");
                return;
            }

            var item = asTimerItem.Item;

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

        private void descriptionTextBox_OnKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                this.setDescriptionIfChanged(this.descriptionTextBox.Text);
                e.Handled = true;
            }
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
            var open = this.projectDropDownButton.IsChecked ?? false;
            if (open)
            {
                this.projectAutoComplete.OpenAndShowAll();
            }
            else
            {
                this.projectAutoComplete.IsOpen = false;
                if (!this.projectTextBox.IsKeyboardFocused)
                {
                    this.projectTextBox.Focus();
                    this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;
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

            this.setProjectIfDifferent(item.TaskID, item.ProjectID, item.ProjectLabel, item.ProjectColor);
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
            if(this.isInNewProjectMode)
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

        private void setProjectIfDifferent(ulong taskId, ulong projectId, string projectName, string projectColor = null)
        {
            if (projectId == this.timeEntry.PID && taskId == this.timeEntry.TID)
                return;
            this.projectTextBox.SetText(projectName);
            this.projectColorCircle.Background = new SolidColorBrush(getProjectColor(projectColor));
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
            this.projectSaveArea.Visibility = Visibility.Visible;
            this.projectTextBox.Focus();
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;

            this.projectColorCircle.Visibility = Visibility.Collapsed;
            this.projectTextBox.Padding = new Thickness(8, 0, 34 + 34, 0);
            this.emptyProjectText.Text = "Add project";
            this.emptyProjectText.Margin = new Thickness(16, 0, 16, 0);

            this.isInNewProjectMode = true;
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
            this.projectSaveArea.Visibility = Visibility.Collapsed;
            this.projectTextBox.Focus();
            this.projectTextBox.CaretIndex = this.projectTextBox.Text.Length;

            this.projectColorCircle.Visibility = Visibility.Visible;
            this.projectTextBox.Padding = new Thickness(28, 0, 34, 0);
            this.emptyProjectText.Text = "No project";
            this.emptyProjectText.Margin = new Thickness(36, 0, 16, 0);

            this.isInNewProjectMode = false;
        }

        private void projectTextBox_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.isInNewProjectMode)
                return;

            switch (e.Key)
            {
                case Key.Escape:
                    {
                        this.disableNewProjectMode();
                        e.Handled = true;
                        break;
                    }
                case Key.Enter:
                    {
                        this.confirmNewProject();
                        e.Handled = true;
                        break;
                    }
            }
        }

        private void confirmNewProject()
        {
            if (this.isInNewClientMode)
            {
                this.tryCreatingNewClient(this.clientTextBox.Text);
            }

            if (this.tryCreatingNewProject(this.projectTextBox.Text))
            {
                this.disableNewProjectMode();
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

        private void projectSaveButton_Click(object sender, RoutedEventArgs e)
        {
            this.confirmNewProject();
        }

        private void projectCancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.disableNewProjectMode();
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
            var open = this.clientDropDownButton.IsChecked ?? false;
            if (open)
            {
                this.clientAutoComplete.OpenAndShowAll();
            }
            else
            {
                this.clientAutoComplete.IsOpen = false;
                if (!this.clientTextBox.IsKeyboardFocused)
                {
                    this.clientTextBox.Focus();
                    this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;
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

        private void selectClient(Toggl.TogglGenericView item)
        {
            this.selectedClientGUID = item.GUID;
            this.selectedClientName = item.Name;
            this.clientTextBox.SetText(item.Name);
        }

        private void clientAutoComplete_OnConfirmWithoutCompletion(object sender, string e)
        {
            if (this.clientTextBox.Text == "")
            {
                this.selectClient(new Toggl.TogglGenericView());
            }
            else
            {
                // TODO: reset client? add new? switch to 'add new client mode'?
            }
        }

        private void clientTextBox_OnLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (this.isInNewClientMode)
                return;

            if (this.clientTextBox.Text == "")
            {
                this.selectClient(new Toggl.TogglGenericView());
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

            this.clientTextBox.Padding = new Thickness(8, 0, 34 + 34, 0);

            this.clientTextBox.Focus();
            this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;
            this.showWorkspaceArea();

            this.emptyClientText.Text = "Add client";

            this.isInNewClientMode = true;
        }

        private void disableNewClientMode()
        {
            this.clientTextBox.SetValue(Grid.ColumnSpanProperty, 1);
            this.clientAutoComplete.IsEnabled = true;
            this.clientDropDownButton.Visibility = Visibility.Visible;
            this.newClientButton.Visibility = Visibility.Visible;
            this.newClientCancelButton.Visibility = Visibility.Hidden;

            this.clientTextBox.Padding = new Thickness(8, 0, 34, 0);

            this.clientTextBox.Focus();
            this.clientTextBox.CaretIndex = this.clientTextBox.Text.Length;
            this.hideWorkspaceArea();

            this.emptyClientText.Text = "No client";

            this.isInNewClientMode = false;
        }

        private void clientTextBox_OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!this.isInNewClientMode)
                return;

            switch (e.Key)
            {
                case Key.Escape:
                    {
                        this.disableNewClientMode();
                        e.Handled = true;
                        break;
                    }
                case Key.Enter:
                    {
                        this.confirmNewClient();
                        e.Handled = true;
                        break;
                    }
            }
        }

        private void confirmNewClient()
        {
            if (this.tryCreatingNewClient(this.clientTextBox.Text))
            {
                this.disableNewClientMode();
                this.projectTextBox.Focus();
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
            var open = this.workspaceDropDownButton.IsChecked ?? false;
            if (open)
            {
                this.workspaceAutoComplete.OpenAndShowAll();
            }
            else
            {
                this.workspaceAutoComplete.IsOpen = false;
                if (!this.workspaceTextBox.IsKeyboardFocused)
                {
                    this.workspaceTextBox.Focus();
                    this.workspaceTextBox.CaretIndex = this.workspaceTextBox.Text.Length;
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

        private void selectWorkspace(Toggl.TogglGenericView item)
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

            this.saveTags();
        }

        private void tagList_OnGotKeybardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            this.updateTagListEmptyText();
        }

        private void tagList_TagAdded(object sender, string e)
        {
            this.saveTags();
        }

        private void tagList_TagRemoved(object sender, string e)
        {
            this.saveTags();
        }

        private void saveTags()
        {
            Toggl.SetTimeEntryTags(this.timeEntry.GUID, this.tagList.Tags.ToList());
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
            timer.StartStopClick += (sender, args) => this.Close();
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Escape:
                {
                    this.Close();
                    e.Handled = true;
                    return;
                }
            }
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
            if (this.isInNewProjectMode)
            {
                this.confirmNewProject();
            }
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
                Toggl.DeleteTimeEntry(this.timeEntry.GUID);
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
